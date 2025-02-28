#ifndef COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_NATIVE_TARGET_H
#define COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_NATIVE_TARGET_H
#include <em_device.h>
#include "em_timer.h"
#include "target_com_sky_nf_dev_c1_config.h"

#ifndef GECKO_USE_C1
#define GECKO_USE_C1    FALSE
#define C1_GPIO_PORT    gpioPortB
#define C1_GPIO_PIN     10
#endif

#if defined(_SILICON_LABS_32B_SERIES_1) && (GECKO_USE_C1 == TRUE)
    #define REGISTER_BIT        (1 << C1_GPIO_PIN)
    #define GPIO_READ()         ((GPIO->P[C1_GPIO_PORT].DIN & REGISTER_BIT) ? 1 : 0)

    // GPIO_SIGNAL(0) = LOW, GPIO_SIGNAL(1) = HIGH
    //#define GPIO_SIGNAL(value)  (value ? (GPIO->P[C1_GPIO_PORT].DOUT |= REGISTER_BIT) : (GPIO->P[C1_GPIO_PORT].DOUT &= ~REGISTER_BIT))
    
    #define GPIO_SIGNAL_BK(state) \
    do { \
        if ((state) == 0) { \
            GPIO->P[1].DOUT &= ~(1 << 9); \
            GPIO->P[1].MODEL = (GPIO->P[1].MODEL & ~(_GPIO_P_MODEL_MODE0_MASK << (9 % 8 * 4))) | (GPIO_P_MODEL_MODE0_PUSHPULL << (9 % 8 * 4)); \
        } else if ((state) == 1) { \
            GPIO->P[1].DOUT |= (1 << 9); \
            GPIO->P[1].MODEL = (GPIO->P[1].MODEL & ~(_GPIO_P_MODEL_MODE0_MASK << (9 % 8 * 4))) | (GPIO_P_MODEL_MODE0_PUSHPULL << (9 % 8 * 4)); \
        } else if ((state) == 2) { \
            GPIO->P[1].MODEL = (GPIO->P[1].MODEL & ~(_GPIO_P_MODEL_MODE0_MASK << (9 % 8 * 4))) | (GPIO_P_MODEL_MODE0_INPUT << (9 % 8 * 4)); \
        } \
    } while (0)

    #define GPIO_SIGNAL(state) \
    do { \
        if ((state) == 0) { \
            GPIO->P[1].DOUT &= ~(1 << 9); \
            GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << (9 % 8 * 4))) | (GPIO_P_MODEH_MODE8_PUSHPULL << (9 % 8 * 4)); \
        } else if ((state) == 1) { \
            GPIO->P[1].DOUT |= (1 << 9); \
            GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << (9 % 8 * 4))) | (GPIO_P_MODEH_MODE8_PUSHPULL << (9 % 8 * 4)); \
        } else if ((state) == 2) { \
            GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << (9 % 8 * 4))) | (GPIO_P_MODEH_MODE8_INPUT << (9 % 8 * 4)); \
        } \
    } while (0)

    // Set the prescaler of timer 0 in order to increase or decrease the frequency
    #define TIMER0_SET_PRESCALER(prescaler) {   \
        TIMER0->CTRL = (TIMER0->CTRL & ~_TIMER_CTRL_PRESC_MASK) | prescaler;   \
        TIMER0->CNT = 0;    \
        TIMER0->CMD = TIMER_CMD_START;  \
        TIMER0->IFC = TIMER_IFC_OF;     \
        TIMER0->IEN = TIMER_IEN_OF;     \
        NVIC->ISER[(((uint32_t)TIMER0_IRQn) >> 5)] = (1 << (((uint32_t)TIMER0_IRQn) & 0x1F));    \
    }
#else
#error                                                                                                                 \
    "Only _SILICON_LABS_32B_SERIES_1 is supported at this time. To add support for other series declaration above has to be updated"
#endif

#endif
