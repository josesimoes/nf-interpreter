/*
//
// Copyright (c) .NET Foundation and Contributors
// Portions Copyright (c) 2006..2015 Giovanni Di Sirio. All rights reserved.
// See LICENSE file in the project root for full license information.
//
*/

/*
 * STM32H5F5xJ generic setup.
 *
 * RAM0 - Stacks, Data, BSS, Heap, CLR managed heap, Ethernet DMA buffers and GPDMA links.
 *
 * Notes:
 * SRAM1 + SRAM2 + SRAM3 are contiguous and are enabled after reset, they are all used as RAM0.
 * SRAM4 and SRAM5 are not used (their clocks need to be enabled before use).
 * The H5 doesn't have a data cache for the internal SRAM, so there is no need to have non cacheable areas.
 * When updating the flash0 address below make sure to update the address in nf_generate_bin_package
 */
MEMORY
{
    flash0       (rx) : org = 0x08020000, len = 2M-64k-64k                /* first bank less the space reserved for nanoBooter and configuration block */
    flash1       (rx) : org = 0x00000000, len = 0
    flash2       (rx) : org = 0x00000000, len = 0
    flash3       (rx) : org = 0x00000000, len = 0
    flash4       (rx) : org = 0x00000000, len = 0
    flash5       (rx) : org = 0x00000000, len = 0
    flash6       (rx) : org = 0x00000000, len = 0
    flash7       (rx) : org = 0x00000000, len = 0
    config       (rw) : org = 0x08010000, len = 64k                       /* space reserved for configuration block (8 sectors of 8kB) */
    deployment   (rx) : org = 0x08200000, len = 2M                        /* space reserved for application deployment (second bank) */
    ramvt        (wx) : org = 0x00000000, len = 0                         /* initial RAM address is reserved for a copy of the vector table */
    ram0         (wx) : org = 0x20000030, len = 768k-48                   /* SRAM1 + SRAM2 + SRAM3 */
    ram1         (wx) : org = 0x20000030, len = 256k-48                   /* SRAM1 */
    ram2         (wx) : org = 0x20040000, len = 128k                      /* SRAM2 */
    ram3         (wx) : org = 0x20060000, len = 384k                      /* SRAM3 */
    ram4         (wx) : org = 0x200C0000, len = 768k                      /* SRAM4 + SRAM5 */
    ram5         (wx) : org = 0x200C0000, len = 384k                      /* SRAM4 */
    ram6         (wx) : org = 0x20120000, len = 384k                      /* SRAM5 */
    ram7         (wx) : org = 0x00000000, len = 0
    bootclpbrd   (wx) : org = 0x20000000, len = 48                        /* boot clipboard area */
}

/* For each data/text section two region are defined, a virtual region
   and a load region (_LMA suffix).*/

/* Flash region to be used for exception vectors.*/
REGION_ALIAS("VECTORS_FLASH", flash0);
REGION_ALIAS("VECTORS_FLASH_LMA", flash0);

/* Flash region to be used for constructors and destructors.*/
REGION_ALIAS("XTORS_FLASH", flash0);
REGION_ALIAS("XTORS_FLASH_LMA", flash0);

/* Flash region to be used for code text.*/
REGION_ALIAS("TEXT_FLASH", flash0);
REGION_ALIAS("TEXT_FLASH_LMA", flash0);

/* Flash region to be used for read only data.*/
REGION_ALIAS("RODATA_FLASH", flash0);
REGION_ALIAS("RODATA_FLASH_LMA", flash0);

/* Flash region to be used for various.*/
REGION_ALIAS("VARIOUS_FLASH", flash0);
REGION_ALIAS("VARIOUS_FLASH_LMA", flash0);

/* Flash region to be used for RAM(n) initialization data.*/
REGION_ALIAS("RAM_INIT_FLASH_LMA", flash0);

/* RAM region to be used for Main stack. This stack accommodates the processing
   of all exceptions and interrupts.*/
REGION_ALIAS("MAIN_STACK_RAM", ram0);

/* RAM region to be used for the process stack. This is the stack used by
   the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", ram0);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", ram0);
REGION_ALIAS("DATA_RAM_LMA", flash0);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", ram0);

/* RAM region to be used for the default heap.*/
REGION_ALIAS("HEAP_RAM", ram0);
REGION_ALIAS("CLR_MANAGED_HEAP_RAM", ram0);

/* RAM region to be used for the boot clipboard.*/
REGION_ALIAS("SECTION_FOR_BOOTCLIPBOARD", bootclpbrd);

/* rules inclusion.*/
INCLUDE rules_stacks.ld

/*===========================================================================*/
/* Custom sections for STM32H5xx.                                            */
/*===========================================================================*/

/* RAM region to be used for the GPDMA links.*/
REGION_ALIAS("DMA3_RAM", ram0);

/* RAM region to be used for eth segment.*/
REGION_ALIAS("ETH_RAM", ram0);

SECTIONS
{
    /* Special section for GPDMA links, it must not exceed 64kB in size and has to be in the first 64kB of RAM.*/
    .dma3 (NOLOAD) : ALIGN(16)
    {
        __dma3_base__ = .;
        *(.dma3)
        *(.dma3.*)
        *(.bss.__dma3_*)
        . = ALIGN(4);
        __dma3_end__ = .;
    } > DMA3_RAM

    /* Special section for Ethernet DMA areas.*/
    .eth (NOLOAD) : ALIGN(4)
    {
        __eth_base__ = .;
        *(.eth)
        *(.eth.*)
        *(.bss.__eth_*)
        . = ALIGN(4);
        __eth_end__ = .;
    } > ETH_RAM
}

/* Code rules inclusion.*/
INCLUDE rules_code.ld

/* Data rules inclusion.*/
INCLUDE rules_data.ld

/* Memory rules inclusion.*/
INCLUDE rules_memory.ld

/* nanoCLR rules inclusion.*/
INCLUDE rules_clr.ld

/* boot clipboard rules inclusion.*/
INCLUDE rules_bootclipboard.ld
