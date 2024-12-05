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
void gpioPinToggle();
void initOscillator();
void cleanUp(); 
void Handle_Instruction_Write(C1InstructionName instructionType, int index, C1_States nextState);
void Handle_Data_State(C1InstructionName instructionType, int index, uint8_t* data, C1_States nextState);
void Handle_Data_Read(int index, uint8_t* data, C1_States nextState);
void Handle_Data_Write(int index, uint8_t* data, C1_States nextState);
void Handle_Address_Read(int index, uint8_t* data, C1_States nextState);
void Handle_Address_Write(int index, uint8_t* data, C1_States nextState);
void Set_Nth_Bit(uint8_t &data, int n, bool logicValue);
void Handle_Data_Read(int index, uint8_t* data);
int Get_Nth_Bit(uint8_t data, int n);
int Get_Instruction_Value(C1InstructionName instructionType, int index);

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

volatile bool toggleGPIO = false;

// keeps count of toggling the GPIO, if even it means we drive low, if odd it means we drive high 
volatile int pinModeToggle = 0;

volatile bool waitForRead = false;

volatile bool pinValue = false;

CMU_Select_TypeDef lastClockSelect;

volatile int holdState = 0;

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
    initOscillator();
    setupGPIO();

    // Reset state machine for the next run currentState = STATE_INIT;
    instruction = DATA_READ;
    currentState = STATE_INIT;
    previousState = STATE_INIT;

    transfer_data = 0;
    bitToRead = 0;

    setupTimer();

    // Wait for the state machine to finish
    while(currentState != STATE_END){}

    param1[0] = 0x01;
    param1[1] = 0x03;

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
    instructions[ADDRESS_READ] = C1Instruction(ADDRESS_READ, 1, 0);

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

    pinModeToggle = 0;
}

void initOscillator(void) 
{
    // lastClockSelect = CMU_ClockSelectGet(cmuClock_HF);

    // // check if oscillator is enabled
    // if((CMU->STATUS & CMU_STATUS_HFXORDY) != 0) {
    //     // Configure the external oscillator
    //     CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    // }
    
    // CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
}

void cleanUp(void)
{
    // CMU_ClockSelectSet(lastClockSelect, cmuSelect_HFXO);
}

void setupTimer()
{
    // // Enable clock for TIMER0
    // CMU_ClockEnable(cmuClock_TIMER0, true);

    // // Get the HFPERB clock frequency
    // uint32_t hfperbFreq = CMU_ClockFreqGet(cmuClock_HFPERB);

    // // Calculate the top value for 2 ns interval
    // uint32_t topValue = (hfperbFreq / 500000000) - 1; // 500 MHz for 2 ns

    // // Configure TIMER0
    // TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    // timerInit.prescale = timerPrescale1; // No prescaling
    // TIMER_Init(TIMER0, &timerInit);

    // // Set TIMER0 top value
    // TIMER_TopSet(TIMER0, topValue);

    // // Enable TIMER0 overflow interrupt
    // TIMER_IntEnable(TIMER0, TIMER_IF_OF);
    // NVIC_EnableIRQ(TIMER0_IRQn);

    // // Start TIMER0
    // TIMER_Enable(TIMER0, true);

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

    // Start TIMER0
    TIMER_Enable(TIMER0, true);

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
            // switch(instruction)
            // {
            //     case DATA_READ:
            //         GPIO_SIGNAL(0);
            //         currentState = INSTRUCTION_2;
            //         break;
            //     default:
            //         currentState = STATE_END;
            //         break;
            // }
            Handle_Instruction_Write(instruction, 0, INSTRUCTION_2);
            break;
        case INSTRUCTION_2:
            // switch(instruction)
            // {
            //     case DATA_READ:
            //         if(holdState++ < 3) {
            //             GPIO_SIGNAL(1);;
            //         }
            //         else {
            //             holdState = 0;
            //             currentState = INCREMENT;
            //         }
            //         break;
            //     default:
            //         currentState = STATE_END;
            //         break;
            // }
            Handle_Instruction_Write(instruction, 1, INCREMENT);
            break;
        case INCREMENT:
            GPIO_SIGNAL(0);
            currentState = READ_1;
            break;
        case READ_1:
            GPIO_SIGNAL(1);
            // GPIO_CONFIGURE_INPUT_PULLUP();
            currentState = READ_2;
            break;
        case READ_2:
            // GPIO_PinOutSet(gpioPort, gpioPin);
            // GPIO_CONFIGURE_INPUT_PULLUP();
            // GPIO_PinModeSet(gpioPort, gpioPin, gpioModeInputPull, 1);
            currentState = READ_3;
            break;
        case READ_3:
            // delay 24 cycles in order to allow the DUT to pull LOW or 
            // release the signal and the pull up to set it HIGH
            for (volatile uint32_t i = 0; i < 4; i++) {
               // Empty loop to create delay
            }

            pinValue = GPIO_READ();
            // GPIO_CONFIGURE_OPEN_DRAIN();
            if(pinValue){
                GPIO_SIGNAL(1);
            }
            else {
                GPIO_SIGNAL(0);
            }

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
            // GPIO_PinModeSet(gpioPort, gpioPin, gpioModeWiredAndPullUp, 1);   // set gpio back to open drain
            
            
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
            readCount = 0;
            Handle_Data_State(instruction, 0, &transfer_data, STATE_END);
            break;
        case DATA_2:
            readCount = 0;
            Handle_Data_State(instruction, 1, &transfer_data, STATE_END);
            break;
        case DATA_3:
            Handle_Data_State(instruction, 2, &transfer_data, DATA_4);
            break;
        case DATA_4:
            Handle_Data_State(instruction, 3, &transfer_data, DATA_5);
            break;
        case DATA_5:
            Handle_Data_State(instruction, 4, &transfer_data, DATA_6);
            break;
        case DATA_6:
            Handle_Data_State(instruction, 5, &transfer_data, DATA_7);
            break;
        case DATA_7:
            Handle_Data_State(instruction, 6, &transfer_data, DATA_8);
            break;
        case DATA_8:
            Handle_Data_State(instruction, 7, &transfer_data, STATE_END);
            currentState = STATE_END;
            break;
        case STATE_END:
            toggleGPIO = false;
            // GPIO_PinOutSet(gpioPort, gpioPin);      // set pin to high
            TIMER_Enable(TIMER0, false);
            break;

        default:
            // Default actions
            TIMER_Enable(TIMER0, false);
            break;  
    }

    if(toggleGPIO) {
        gpioPinToggle();
    }

    
}

void gpioPinToggle() 
{
    if(pinModeToggle % 2 == 0) {
        GPIO_PinOutClear(gpioPort, gpioPin);    // drives pin low
    }
    else {
        GPIO_PinOutSet(gpioPort, gpioPin);      // drives pin high (with internal pull-up)
    }

    pinModeToggle++;
}

void Handle_Instruction_Write(C1InstructionName instructionType, int index, C1_States nextState)
{
    // C1Instruction* inst = fetchInstruction(instructions, instructionType);

    // if(inst == nullptr) {
    //     return;
    // }

    int instr_value = Get_Instruction_Value(instructionType, index);

    if(instr_value == 1 && writeLogical1 < 3) {
        GPIO_SIGNAL(1);
        writeLogical1++;
    }
    else {
        GPIO_SIGNAL(0);
        currentState = nextState;
        writeLogical1 = 0;
    }
}

int Get_Instruction_Value(C1InstructionName instructionType, int index)
{
    instruction = instructionType;
    index = 0;

    return -1;
}

void Handle_Data_Read(int index, uint8_t* data)
{
    switch(index){
        case 0:
            toggleGPIO = true;
            break;
        case 1:
            toggleGPIO = false;
            break;
        case 2:
            toggleGPIO = false;
            // Release the GPIO pin
            GPIO_PinModeSet(gpioPort, gpioPin, gpioModeInputPull, 1);
            pinValue = GPIO_PinInGet(gpioPort, gpioPin) == 0;
            Set_Nth_Bit(*data, index, pinValue);
            break;
        case 3:
            toggleGPIO = false;
            GPIO_PinModeSet(gpioPort, gpioPin, gpioModeWiredAndPullUp, 1);   // set gpio back to open drain
            if(pinValue){
                GPIO_PinOutClear(gpioPort, gpioPin);
            }
            else {
                GPIO_PinOutSet(gpioPort, gpioPin);
            }
            break;
        case 4:
            toggleGPIO = false;
            break;
        case 5:
            if(pinValue)
            {
                GPIO_PinOutSet(gpioPort, gpioPin);
            }
            break;
    }
}

void Handle_Data_State(C1InstructionName instructionType, int index, uint8_t* data, C1_States nextState) 
{
    if(instructionType == DATA_READ) {
        Handle_Data_Read(index, data, nextState);
    }
    else if(instructionType == DATA_WRITE) {
        Handle_Data_Write(index, data, nextState);
    }
    else if(instructionType == ADDRESS_READ) {
        Handle_Address_Read(index, data, nextState);
    }
    else if(instructionType == ADDRESS_WRITE) {
        Handle_Address_Write(index, data, nextState);
    }
}

void Handle_Data_Read(int index, uint8_t* data, C1_States nextState)
{
    if(readCount == 0) {
        // do nothing
        toggleGPIO = true;
    }
    else if(readCount == 1) {
        // tell state machine to not toggle gpio and release it to allow
        // slave to drive high or low
        toggleGPIO = false;
    }
    else if(readCount == 2) {
        // read the value of the gpio
        toggleGPIO = false;
        if(!waitForRead) {
            waitForRead = true;
            bool pinValue = GPIO_PinInGet(gpioPort, gpioPin) == 0;
            Set_Nth_Bit(*data, index, pinValue);
            waitForRead = false;
        }
    }
    else if(readCount == 3 || readCount == 4 || readCount == 5) {
        toggleGPIO = false;
    }
    else if(readCount == 6) {
        // do not toggle, but drive pin low
        toggleGPIO = false;
        GPIO_PinOutClear(gpioPort, gpioPin);
    }

    if(readCount < 6) {
        if(!waitForRead) {
            readCount++;
        }
    } else {
        readCount = 0;
        currentState = nextState;
        pinModeToggle = 0;  // ensure next pin toggle set gpio to HIGH
    }
}

void Handle_Data_Write(int index, uint8_t* data, C1_States nextState)
{
    int write_bit = Get_Nth_Bit(*data, index);

    if(write_bit == 1 && writeLogical1 < 3) {
        writeLogical1++;
        toggleGPIO = false;
    }
    else {
        writeLogical1 = 0;
        toggleGPIO = true;
        currentState = nextState;
    }
}

void Handle_Address_Read(int index, uint8_t* data, C1_States nextState)
{
    index++;
    data[0] = 1;
    currentState = nextState;
}

void Handle_Address_Write(int index, uint8_t* data, C1_States nextState)
{
    index++;
    data[0] = 1;
    currentState = nextState;
}

int Get_Nth_Bit(uint8_t data, int n)
{
    if(n < 8) {
        if((data & (1 << n)) != 0) {
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


