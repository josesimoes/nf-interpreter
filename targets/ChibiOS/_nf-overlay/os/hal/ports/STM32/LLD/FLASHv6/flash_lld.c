//
// Copyright (c) .NET Foundation and Contributors
// Portions Copyright (c) 2017 STMicroelectronics.  All rights reserved.
// See LICENSE file in the project root for full license information.
//

///////////////////////////////////////////////////////////////////////////////
// This driver has been tested with the following STM32 series: H5 (non secure, TrustZone disabled)
///////////////////////////////////////////////////////////////////////////////

#include <hal.h>
#include <hal_nf_community.h>

#if (HAL_NF_USE_STM32_FLASH == TRUE)

///////////////////////////////////////////////////////////////////////////////
// Driver local definitions.                                                 //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Driver exported variables.                                                //
///////////////////////////////////////////////////////////////////////////////
STM32FlashDriver STM32FLASH;

///////////////////////////////////////////////////////////////////////////////
// Driver local variables and types.                                         //
///////////////////////////////////////////////////////////////////////////////

// timeout for a flash operation (sector erase is the longest one)
#define FLASH_OPERATION_TIMEOUT_MS 1000U

// number of loops to wait for the instruction cache invalidation to complete
#define FLASH_ICACHE_INVALIDATE_TIMEOUT 100000U

// buffer for a flash word
static union
{
    uint32_t w[STM32_FLASH_WORDS_PER_LINE];
    uint8_t b[STM32_FLASH_LINE_SIZE];
} line;

///////////////////////////////////////////////////////////////////////////////
// Driver local functions.                                                   //
///////////////////////////////////////////////////////////////////////////////

static bool FLASH_Unlock(void)
{
    if ((FLASH->NSCR & FLASH_CR_LOCK) != 0U)
    {
        // Authorize the FLASH control register access
        FLASH->NSKEYR = FLASH_KEY1;
        FLASH->NSKEYR = FLASH_KEY2;

        // check that unlock succeeded
        if ((FLASH->NSCR & FLASH_CR_LOCK) != 0U)
        {
            return false;
        }
    }

    return true;
}

static void FLASH_Lock(void)
{
    // Set the LOCK Bit to lock the FLASH control register access
    FLASH->NSCR |= FLASH_CR_LOCK;
}

static void FLASH_ClearErrors(void)
{
    FLASH->NSCCR = FLASH_CLEAR_ALL_ERRORS | FLASH_CCR_CLR_EOP;
}

static bool FLASH_WaitForLastOperation(void)
{
    // Wait for the FLASH operation to complete by polling on BSY flag to be reset.
    // WBNE is also checked to make sure that the write buffer has been flushed.
    // The timeout has to be longer than the worst case sector erase time
    systime_t start = osalOsGetSystemTimeX();

    while ((FLASH->NSSR & (FLASH_SR_BSY | FLASH_SR_WBNE)) != 0U)
    {
        if (osalTimeDiffX(start, osalOsGetSystemTimeX()) > OSAL_MS2I(FLASH_OPERATION_TIMEOUT_MS))
        {
            return false;
        }
    }

    if ((FLASH->NSSR & FLASH_FLAG_ALL_ERRORS) != 0U)
    {
        FLASH_ClearErrors();
        return false;
    }

    // Clear FLASH End of Operation pending bit
    if ((FLASH->NSSR & FLASH_SR_EOP) != 0U)
    {
        FLASH->NSCCR = FLASH_CCR_CLR_EOP;
    }

    return true;
}

static void FLASH_InvalidateCache(void)
{
    // flash contents changed behind the instruction cache back (it is used for data reads from flash too)
    if ((ICACHE->CR & ICACHE_CR_EN) != 0U)
    {
        uint32_t timeout = FLASH_ICACHE_INVALIDATE_TIMEOUT;

        // clear any previous busy end flag
        ICACHE->FCR = ICACHE_FCR_CBSYENDF;

        // start the invalidation and wait for it to complete
        ICACHE->CR |= ICACHE_CR_CACHEINV;

        while (((ICACHE->SR & ICACHE_SR_BSYENDF) == 0U) && (timeout > 0U))
        {
            timeout--;
        }

        ICACHE->FCR = ICACHE_FCR_CBSYENDF;
    }

    __DSB();
    __ISB();
}

///////////////////////////////////////////////////////////////////////////////
// Driver exported functions.                                                //
///////////////////////////////////////////////////////////////////////////////

void flash_lld_init()
{
    stm32FlashObjectInit(&STM32FLASH);
}

void flash_lld_readBytes(uint32_t startAddress, uint32_t length, uint8_t *buffer)
{
    __IO uint8_t *cursor = (__IO uint8_t *)startAddress;
    __IO uint8_t *endAddress = (__IO uint8_t *)(startAddress + length);

    // copy contents from flash to buffer starting from the start address
    while (cursor < endAddress)
    {
        *buffer++ = *cursor++;
    }
}

// H5 flash is programmed by flash words (128 bits) with ECC, which means that a flash word can be programmed only once
// after an erase. Partial writes are merged with the current flash word contents. Writing to a flash word that is
// already programmed is only allowed if the contents don't change.
int flash_lld_write(uint32_t startAddress, uint32_t length, const uint8_t *buffer)
{
    uint32_t offset = startAddress;
    uint32_t remainingBytes = length;
    bool success = true;
    uint32_t flashEnd = FLASH_BASE + FLASH_SIZE - 1U;

    // the range to write has to be inside the flash (the comparisons are written so they can't overflow)
    if (startAddress < FLASH_BASE || startAddress > flashEnd || length > flashEnd - startAddress + 1U)
    {
        return false;
    }

    if (!FLASH_Unlock())
    {
        return false;
    }

    FLASH_ClearErrors();

    while (remainingBytes > 0U)
    {
        // Programming address aligned to flash words
        volatile uint32_t *address = (volatile uint32_t *)(offset & ~STM32_FLASH_LINE_MASK);
        bool isErased = true;
        bool isChanged = false;

        // load current flash word contents
        for (uint32_t i = 0; i < STM32_FLASH_WORDS_PER_LINE; i++)
        {
            line.w[i] = address[i];

            if (line.w[i] != STM32_FLASH_ERASED_WORD)
            {
                isErased = false;
            }
        }

        // copy data inside the prepared flash word
        do
        {
            if (line.b[offset & STM32_FLASH_LINE_MASK] != *buffer)
            {
                line.b[offset & STM32_FLASH_LINE_MASK] = *buffer;
                isChanged = true;
            }

            offset++;
            remainingBytes--;
            buffer++;
        } while ((remainingBytes > 0U) && ((offset & STM32_FLASH_LINE_MASK) != 0U));

        if (!isChanged)
        {
            // nothing to program here
            continue;
        }

        if (!isErased)
        {
            // can't program a flash word twice
            success = false;
            break;
        }

        // proceed to program the flash by setting the PG Bit
        SET_BIT(FLASH->NSCR, FLASH_CR_PG);

        __ISB();
        __DSB();

        // program the flash word
        for (uint32_t i = 0; i < STM32_FLASH_WORDS_PER_LINE; i++)
        {
            address[i] = line.w[i];
        }

        __ISB();
        __DSB();

        success = FLASH_WaitForLastOperation();

        // after the program operation is completed disable the PG Bit
        CLEAR_BIT(FLASH->NSCR, FLASH_CR_PG);

        if (!success)
        {
            // quit on failure
            break;
        }
    }

    FLASH_Lock();

    FLASH_InvalidateCache();

    return success;
}

int flash_lld_isErased(uint32_t startAddress, uint32_t length)
{
    __IO uint32_t *cursor = (__IO uint32_t *)startAddress;
    __IO uint32_t *endAddress = (__IO uint32_t *)(startAddress + length);

    // an erased flash address has to read STM32_FLASH_ERASED_WORD
    // OK to check by word (32 bits) because the erase is performed by 'sector' whose size is word multiple
    while (cursor < endAddress)
    {
        if (*cursor++ != STM32_FLASH_ERASED_WORD)
        {
            // found an address with something other than STM32_FLASH_ERASED_WORD!!
            return false;
        }
    }

    // reached here so the segment must be erased
    return true;
}

// gets the sector number counting from the start of the flash, the sector numbers in the second bank follow the first
uint16_t flash_lld_getSector(uint32_t address)
{
    return (uint16_t)((address - FLASH_BASE) / FLASH_SECTOR_SIZE);
}

int flash_lld_erase(uint32_t address)
{
    bool success;
    uint32_t bank;
    uint32_t sector;

    // the address has to be inside the flash
    if (address < FLASH_BASE || (address - FLASH_BASE) >= FLASH_SIZE)
    {
        return false;
    }

    // the flash has two banks with the same size, the sectors are numbered by bank
    bank = (address - FLASH_BASE) / FLASH_BANK_SIZE;
    sector = ((address - FLASH_BASE) % FLASH_BANK_SIZE) / FLASH_SECTOR_SIZE;

    if (!FLASH_Unlock())
    {
        return false;
    }

    FLASH_ClearErrors();

    // erase sector
    MODIFY_REG(
        FLASH->NSCR,
        (FLASH_CR_SNB | FLASH_CR_BKSEL),
        (FLASH_CR_SER | (sector << FLASH_CR_SNB_Pos) | ((bank != 0U) ? FLASH_CR_BKSEL : 0U)));

    // start erase operation
    FLASH->NSCR |= FLASH_CR_START;

    __DSB();

    success = FLASH_WaitForLastOperation();

    // after erase operation completed disable the SER, SNB and BKSEL Bits
    CLEAR_BIT(FLASH->NSCR, (FLASH_CR_SER | FLASH_CR_SNB | FLASH_CR_BKSEL));

    FLASH_Lock();

    FLASH_InvalidateCache();

    return success;
}

#endif
