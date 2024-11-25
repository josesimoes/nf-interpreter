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
void Handle_Instruction_Write(C1InstructionName instructionType, int index, C1_States nextState);
void Handle_Data_State(C1InstructionName instructionType, int index, uint8_t* data, C1_States nextState);
void Handle_Data_Read(int index, uint8_t* data, C1_States nextState);
void Handle_Data_Write(int index, uint8_t* data, C1_States nextState);
void Handle_Address_Read(int index, uint8_t* data, C1_States nextState);
void Handle_Address_Write(int index, uint8_t* data, C1_States nextState);
void Set_Nth_Bit(uint8_t &data, int n, bool logicValue);
int Get_Nth_Bit(uint8_t data, int n);

// current and previous state place holders
volatile enum C1_States currentState = STATE_INIT;
volatile enum C1_States previousState = STATE_INIT;

GPIO_Port_TypeDef gpioPort = gpioPortB;
unsigned int gpioPin = 10;

C1InstructionName instruction;

uint8_t transfer_data = 0;

// hold the instruction values for C1 protocol
std::map<C1InstructionName, C1Instruction> instructions;

// allow us to only run init function once
int ignoreInit = 0;

// Logical 1 is three times the duration of logical 0, therefore we need to wait 3 periods
// we will use this variable as a counter for the number of periods we have waited.
int writeLogical1 = 0;

// Count to help iterate the read sequence of a bit
int readCount = 0;

bool toggleGPIO = false;

// keeps count of toggling the GPIO, if even it means we drive low, if odd it means we drive high 
int pinModeToggle = 0;

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

    // Reset state machine for the next run currentState = STATE_INIT;
    instruction = DATA_READ;
    currentState = STATE_INIT;
    previousState = STATE_INIT;

    setupTimer();

    // Wait for the state machine to finish
    while(currentState == STATE_END){}

    param1[0] = 0x01;
    param1[1] = 0x03;

    // implementation ends here   //
    ////////////////////////////////
}

void init()
{
    if(ignoreInit > 0) {
        return;
    }

    // Define the instructions
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

void setupTimer()
{
    // Enable clock for TIMER0
    CMU_ClockEnable(cmuClock_TIMER0, true);

    // Initialize the TIMER
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    timerInit.prescale = timerPrescale1;
    TIMER_Init(TIMER0, &timerInit);

    // Set the TIMER Top value for 80 microseconds period
    uint32_t timerFreq = CMU_ClockFreqGet(cmuClock_HFPER);
    uint32_t topValue = (timerFreq / 1e9) * 200; // 200 ns
    TIMER_TopSet(TIMER0, topValue);

    // Enable TIMER0 interrupt
    TIMER_IntEnable(TIMER0, TIMER_IEN_OF);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler()
{
    // Clear TIMER0 interrupt flag
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

    previousState = currentState;

    switch (currentState)
    {
        case STATE_INIT:
            currentState = START_1;
            break;
        case START_1:
            currentState = START_2;
            break;
        case START_2:
            currentState = ADDRESS_FOLLOW;
            break;
        case ADDRESS_FOLLOW:
            currentState = SLOW_ACCESS;
            break;
        case SLOW_ACCESS:
            currentState = INSTRUCTION_1;
            break;
        case INSTRUCTION_1:
            Handle_Instruction_Write(instruction, 0, INSTRUCTION_2);
            break;
        case INSTRUCTION_2:
            Handle_Instruction_Write(instruction, 0, INCREMENT);
            break;
        case INCREMENT:
            currentState = DATA_1;
            break;
        case DATA_1:
            Handle_Data_State(instruction, 0, &transfer_data, DATA_2);
            break;
        case DATA_2:
            Handle_Data_State(instruction, 1, &transfer_data, DATA_3);
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
            TIMER_Enable(TIMER0, false);
            break;

        default:
            // Default actions
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
        GPIO_PinOutSet(gpioPort, gpioPin); // drives pin high (with internal pull-up)
    }

    pinModeToggle++;
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
        bool pinValue = GPIO_PinInGet(gpioPort, gpioPin) == 0;
        Set_Nth_Bit(*data, index, pinValue);
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
        readCount++;
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

void Handle_Instruction_Write(C1InstructionName instructionType, int index, C1_States nextState)
{
    C1Instruction* inst = fetchInstruction(instructions, instructionType);

    if(inst == nullptr) {
        return;
    }

    if(inst->states[index] == 1 && writeLogical1 < 3) {
        writeLogical1++;
        toggleGPIO = false;
    }
    else {
        writeLogical1 = 0;
        toggleGPIO = true;
        currentState = nextState;
    }
}
