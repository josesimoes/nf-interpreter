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
#include "Com_SkyworksInc_NanoFramework_Devices_C1.h"
#include "Com_SkyworksInc_NanoFramework_Devices_C1_Com_SkyworksInc_NanoFramework_Devices_C1_C1Bus.h"

using namespace Com_SkyworksInc_NanoFramework_Devices_C1::Com_SkyworksInc_NanoFramework_Devices_C1;

void setupTimer();
void setupGPIO();
void TIMER0_IRQHandler();

enum C1_States
{
    STATE_INIT,
    STATE_RUN,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    T8,
    T9,
    TREAD_1,
    TREAD_2,
    TREAD_3,
    TREAD_4,
    TREAD_5,
    STATE_END
};

// Current state
volatile enum C1_States currentState = STATE_INIT;

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

    setupGPIO();

    // Reset state machine for the next run currentState = STATE_INIT;
    currentState = STATE_INIT;

    setupTimer();

    // Wait for the state machine to finish
    while(currentState == STATE_END){}

    param1[0] = 0x01;
    param1[1] = 0x03;

    // implementation ends here   //
    ////////////////////////////////
}

void setupGPIO()
{
    // Enable clock for GPIO
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Configure pin as push-pull output
    GPIO_PinModeSet(gpioPortB, 10, gpioModeWiredAndPullUp, 0);
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
    uint32_t topValue = (timerFreq / 1e6) * 80; // 80 microseconds
    TIMER_TopSet(TIMER0, topValue);

    // Enable TIMER0 interrupt
    TIMER_IntEnable(TIMER0, TIMER_IEN_OF);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler()
{
    // Clear TIMER0 interrupt flag
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

    // State machine
    static int toggleCount = 0;

    switch (currentState)
    {
        case STATE_INIT:
            // Perform initialization actions
            // Transition to next state
            toggleCount = 0;
            currentState = STATE_RUN;
            break;
        case STATE_RUN:
            GPIO_PinOutToggle(gpioPortB, 10);

            if(++toggleCount >= 5)
            {
                currentState = STATE_END;
            }
            break;
        case STATE_END:
            TIMER_Enable(TIMER0, false);
            break;

        default:
            // Default actions
            break;  
    }
}
