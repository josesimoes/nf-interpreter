#ifndef COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_NATIVE_TARGET_H
#define COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_NATIVE_TARGET_H
#include <em_device.h>
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
    #define GPIO_SIGNAL(value)  (value ? (GPIO->P[C1_GPIO_PORT].DOUT |= REGISTER_BIT) : (GPIO->P[C1_GPIO_PORT].DOUT &= ~REGISTER_BIT))
#else
#error                                                                                                                 \
    "Only _SILICON_LABS_32B_SERIES_1 is supported at this time. To add support for other series declaration above has to be updated"
#endif

#endif
