//-----------------------------------------------------------------------------
//
//                   ** WARNING! **
//    This file was generated automatically by a tool.
//    Re-running the tool will overwrite this file.
//    You should copy this file to a custom location
//    before adding any customization in the copy to
//    prevent loss of your changes when the tool is
//    re-run.
//
//-----------------------------------------------------------------------------

#include "sl_sleeptimer.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include <map>
#include <Com_SkyworksInc_NanoFramework_Devices_C1_C1Instruction.h>
#include "Com_SkyworksInc_NanoFramework_Devices_C1.h"
#include "Com_SkyworksInc_NanoFramework_Devices_C1_Com_SkyworksInc_NanoFramework_Devices_C1_C1Bus.h"

using namespace Com_SkyworksInc_NanoFramework_Devices_C1::Com_SkyworksInc_NanoFramework_Devices_C1;

enum C1_States
{
    STATE_INIT,
    STATE_RUN,
    START_1,
    START_2,
    ADDRESS_FOLLOW,     // allows multiple devices on C1 line, but we only use one, so always 0
    SLOW_ACCESS,        // Newer chips (from Si5302 on) only use FastPath so must be set to 0
    INSTRUCTION_1,
    INSTRUCTION_2,
    INCREMENT,
    READ_1,
    READ_2,
    READ_3,
    READ_4,
    READ_5,
    READ_6,
    DATA_1,
    DATA_2,
    DATA_3,
    DATA_4,
    DATA_5,
    DATA_6,
    DATA_7,
    DATA_8,
    STATE_END
};

void init();
void setupTimer();
void setupGPIO();
void TIMER0_IRQHandler();
void cleanUp(); 
void Handle_Instruction_Write(C1InstructionName instructionType, int index, C1_States nextState);
void Handle_Data_Write(int index, uint8_t* data, C1_States nextState);
void Set_Nth_Bit(uint8_t &data, int n, bool logicValue);
int Get_Nth_Bit(uint8_t data, int n);

// current and previous state place holders
volatile enum C1_States currentState = STATE_INIT;
volatile enum C1_States previousState = STATE_INIT;

GPIO_Port_TypeDef gpioPort = gpioPortB;
unsigned int gpioPin = 10;

volatile C1InstructionName instruction;

uint8_t transfer_data = 0;

// hold the instruction values for C1 protocol
C1Instruction instructions[4];

// allow us to only run init function once
volatile int ignoreInit = 0;

// Logical 1 is three times the duration of logical 0, therefore we need to wait 3 periods
// we will use this variable as a counter for the number of periods we have waited.
volatile int writeLogical1 = 0;

// Count to help iterate the read sequence of a bit
volatile int readCount = 0;

volatile bool pinValue = false;

CMU_Select_TypeDef lastClockSelect;

volatile int bitToRead = 0;

volatile int tRest = 0;

void C1Bus::NativeTransmitWrite(uint8_t param0, uint8_t param1, CLR_RT_TypedArray_UINT8 param2, HRESULT &hr)
{

    (void)param0;
    (void)param1;
    (void)param2;
    (void)hr;

    ////////////////////////////////
    // implementation starts here //

    // param0 is register address
    // param1 is write value for address
    // param2 is response array

    // implementation ends here   //
    ////////////////////////////////
}

void C1Bus::NativeTransmitRead(uint8_t param0, CLR_RT_TypedArray_UINT8 param1, HRESULT &hr)
{

    (void)param0;
    (void)param1;
    (void)hr;

    ////////////////////////////////
    // implementation starts here //

    // param0 is register address
    // param1 is response array

    init();
    setupGPIO();

    transfer_data = 0;
    bitToRead = 0;

    setupTimer();

    /*
     *  First address write
     */
    // Reset state machine for the next run currentState = STATE_INIT;
    instruction = ADDRESS_WRITE;
    currentState = STATE_INIT;
    previousState = STATE_INIT;
    transfer_data = param0;
    TIMER_Enable(TIMER0, true);     // Start TIMER0

    // Wait for the state machine to finish
    while(currentState != STATE_END){}

    // Wait in between Address Write and Data Read

    for (volatile uint32_t i = 0; i < 64; i++) {
        // Empty loop to create delay
    }

    /*
     *  Second data read
     */
    // Reset state machine for the next run currentState = STATE_INIT;
    instruction = DATA_READ;
    currentState = STATE_INIT;
    previousState = STATE_INIT;
    transfer_data = 0;
    TIMER_Enable(TIMER0, true);     // Start TIMER0

    // Wait for the state machine to finish
    while(currentState != STATE_END){}

    param1[0] = 0x01;
    param1[1] = transfer_data;

    cleanUp();

    // implementation ends here   //
    ////////////////////////////////
}

void init()
{
    if(ignoreInit > 0) {
        return;
    }

    instructions[DATA_WRITE] = C1Instruction(DATA_WRITE, 0, 0);
    instructions[DATA_READ] = C1Instruction(DATA_READ, 0, 1);
    instructions[ADDRESS_WRITE] = C1Instruction(ADDRESS_WRITE, 1, 0);
    instructions[ADDRESS_READ] = C1Instruction(ADDRESS_READ, 1, 1);

    ignoreInit++;
}

void setupGPIO()
{
    gpioPort = gpioPortB;
    gpioPin = 10;

    // Enable clock for GPIO
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Configure pin as push-pull output, the third argument makes it so when the gpio is CLEAR it
    // goes LOW and when SET it releases the pin the pull-up will drive it HIGH
    GPIO_PinModeSet(gpioPort, gpioPin, gpioModeWiredAndPullUp, 1);

    // Set the GPIO to HIGH
    GPIO_PinOutSet(gpioPort, gpioPin);
}

void cleanUp(void)
{
    // CMU_ClockSelectSet(lastClockSelect, cmuSelect_HFXO);
}

void setupTimer()
{
    // Enable clock for TIMER0 and the HFPERB clock branch
    CMU_ClockEnable(cmuClock_TIMER0, true);
    CMU_ClockEnable(cmuClock_HFPERB, true);

    // Timer initialization structure
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;

    // Configure TIMER0
    timerInit.enable     = false;    // Do not enable timer immediately
    timerInit.prescale   = timerPrescale16;  // Set prescaler (adjust as needed)
    timerInit.clkSel     = timerClkSelHFPerClk;  // Use HFPERB clock branch
    timerInit.mode       = timerModeUp; // Up-counting mode

    TIMER_Init(TIMER0, &timerInit);

    // Set TIMER0 Top value for desired interrupt frequency
    uint32_t timerFreq = CMU_ClockFreqGet(cmuClock_HFPERB) / (1 << timerInit.prescale);
    uint32_t topValue = (timerFreq / 500000) - 1; // 500 kHz for 2 microseconds
    TIMER_TopSet(TIMER0, topValue);  // Example: 1 kHz interrupt rate

    // Enable TIMER0 interrupts
    TIMER_IntEnable(TIMER0, TIMER_IF_OF);  // Enable overflow interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);

    

}

void TIMER0_IRQHandler()
{
    // Clear TIMER0 interrupt flag
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

    switch (currentState)
    {
        case STATE_INIT:
            currentState = START_1;
            break;
        case START_1:
            // GPIO_PinOutClear(gpioPort, gpioPin);
            GPIO_SIGNAL(0);
            currentState = START_2;
            break;
        case START_2:
            GPIO_SIGNAL(1);
            currentState = ADDRESS_FOLLOW;
            break;
        case ADDRESS_FOLLOW:
            GPIO_SIGNAL(0);
            currentState = SLOW_ACCESS;
            break;
        case SLOW_ACCESS:
            GPIO_SIGNAL(1);
            currentState = INSTRUCTION_1;
            break;
        case INSTRUCTION_1:
            Handle_Instruction_Write(instruction, 0, INSTRUCTION_2);
            break;
        case INSTRUCTION_2:
            Handle_Instruction_Write(instruction, 1, INCREMENT);
            break;
        case INCREMENT:
            GPIO_SIGNAL(0);

            // Determine the next state
            switch(instruction) {
                case DATA_READ:
                case ADDRESS_READ:
                    currentState = READ_1;
                    break;
                case DATA_WRITE:
                case ADDRESS_WRITE:
                    currentState = DATA_1;
                    break;
            }
            break;
        case READ_1:
            GPIO_SIGNAL(1);
            currentState = READ_2;
            break;
        case READ_2:
            currentState = READ_3;
            break;
        case READ_3:
            // delay 4 cycles in order to allow the DUT to pull LOW or 
            // release the signal and the pull up to set it HIGH
            for (volatile uint32_t i = 0; i < 4; i++) {
               // Empty loop to create delay
            }

            pinValue = GPIO_READ();
            
            // keep the signal LOW or HIGH depending on what the DUT signal was
            if(pinValue){
                GPIO_SIGNAL(1);
            }
            else {
                GPIO_SIGNAL(0);
            }

            // modify the current bit of transfer_data which holds the register
            // value
            if(pinValue) {
                transfer_data |= (1 << bitToRead);
            }
            else {
                transfer_data &= ~(1 << bitToRead);
            }

            bitToRead++;
            currentState = READ_4;
            break;
        case READ_4:
            currentState = READ_5;
            break;
        case READ_5:
            // if value is 1 we need to pull the line low for an extra cycle in READ_6
            if(pinValue) {
                currentState = READ_6;
            }
            else {
                if(tRest < 1) {
                    tRest++;
                }
                else {
                    // byte value is 0 and there's still values to read go back to
                    // READ_1, if not go to STATE_END
                    if(bitToRead < 8) {
                        currentState = READ_1;
                    }
                    else {
                        currentState = STATE_END;
                    }
                }
            }
                
            break;
        case READ_6:
            // set pin to low
            GPIO_SIGNAL(0);

            // stay at this state for 2 cycles
            if(tRest < 1) {
                tRest++;
            }
            else {
                tRest = 0;
                // if there's more bits to read go back to READ_1
                // else go to end
                if(bitToRead < 8){
                    currentState = READ_1;
                }
                else {
                    currentState = STATE_END;
                }
            }

            
            break;
        case DATA_1:
            Handle_Data_Write(0, &transfer_data, DATA_2);
            currentState = DATA_2;
            break;
        case DATA_2:
            Handle_Data_Write(1, &transfer_data, DATA_3);
            currentState = DATA_3;
            break;
        case DATA_3:
            Handle_Data_Write(2, &transfer_data, DATA_4);
            currentState = DATA_4;
            break;
        case DATA_4:
            Handle_Data_Write(3, &transfer_data, DATA_5);
            currentState = DATA_5;
            break;
        case DATA_5:
            Handle_Data_Write(4, &transfer_data, DATA_6);\
            currentState = DATA_6;
            break;
        case DATA_6:
            Handle_Data_Write(5, &transfer_data, DATA_7);
            currentState = DATA_7;
            break;
        case DATA_7:
            Handle_Data_Write(6, &transfer_data, DATA_8);
            currentState = DATA_8;
            break;
        case DATA_8:
            Handle_Data_Write(7, &transfer_data, STATE_END);
            currentState = STATE_END;
            break;
        case STATE_END:
            GPIO_SIGNAL(1);
            TIMER_Enable(TIMER0, false);
            break;

        default:
            // Default actions
            TIMER_Enable(TIMER0, false);
            break;  
    }

    
}

void Handle_Instruction_Write(C1InstructionName instructionType, int index, C1_States nextState)
{
    int instr_value = instructions[instructionType].states[index];

    if((index % 2) == 0) {
        GPIO_SIGNAL(0);
    }
    else {
        GPIO_SIGNAL(1);
    }

    if(instr_value == 1 && writeLogical1 < 3) {
        writeLogical1++;
    }
    else {
        currentState = nextState;
        writeLogical1 = 0;
    }
}

void Handle_Data_Write(int index, uint8_t* data, C1_States nextState)
{
    int write_bit = Get_Nth_Bit(*data, index);

    if((index % 2) == 0) {
        GPIO_SIGNAL(1);
    }
    else {
        GPIO_SIGNAL(0);
    }
 
    if(write_bit == 1 && writeLogical1 < 3) {
        writeLogical1++;
    }
    else {
        writeLogical1 = 0;
        currentState = nextState;
    }
}

int Get_Nth_Bit(uint8_t data, int n)
{
    if(n < 8) {
        if((data & (1 << n)) == 0) {
            return 0;
        } else {
            return 1;
        }
    }

    return -1;
}

void Set_Nth_Bit(uint8_t &data, int n, bool logicValue)
{
    if(n < 8) {
        if(logicValue) {
            data |= (1 << n);
        }
        else {
            data &= ~(1 << n);
        }
    }
}


