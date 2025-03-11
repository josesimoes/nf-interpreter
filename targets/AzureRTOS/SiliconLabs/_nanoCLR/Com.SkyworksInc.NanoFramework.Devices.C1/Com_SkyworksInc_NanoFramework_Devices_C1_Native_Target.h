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
    #define GPIO_SIGNAL(state) \
    do { \
        if ((state) == 0) { \
            GPIO->P[C1_GPIO_PORT].DOUT &= ~(1 << C1_GPIO_PIN); \
            GPIO->P[C1_GPIO_PORT].MODEH = (GPIO->P[C1_GPIO_PORT].MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << (C1_GPIO_PIN % 8 * 4))) | (GPIO_P_MODEH_MODE8_PUSHPULL << (C1_GPIO_PIN % 8 * 4)); \
        } else if ((state) == 1) { \
            GPIO->P[C1_GPIO_PORT].DOUT |= (1 << C1_GPIO_PIN); \
            GPIO->P[C1_GPIO_PORT].MODEH = (GPIO->P[C1_GPIO_PORT].MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << (C1_GPIO_PIN % 8 * 4))) | (GPIO_P_MODEH_MODE8_PUSHPULL << (C1_GPIO_PIN % 8 * 4)); \
        } else if ((state) == 2) { \
            GPIO->P[C1_GPIO_PORT].MODEH = (GPIO->P[C1_GPIO_PIN].MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << (C1_GPIO_PIN % 8 * 4))) | (GPIO_P_MODEH_MODE8_INPUT << (C1_GPIO_PIN % 8 * 4)); \
        } \
    } while (0)
#else
#error                                                                                                                 \
    "Only _SILICON_LABS_32B_SERIES_1 is supported at this time. To add support for other series declaration above has to be updated"
#endif

#endif
