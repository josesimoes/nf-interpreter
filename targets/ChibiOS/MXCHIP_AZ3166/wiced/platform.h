//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// AZ3166 platform definitions for WICED WWD.
// Defines the GPIO/pin assignments for the BCM43362 WiFi module
// on the MXCHIP EMW3166 (as used on the AZ3166 DevKit board).

#ifndef INCLUDED_PLATFORM_H_
#define INCLUDED_PLATFORM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// Pin definitions for BCM43362 WiFi module on MXCHIP AZ3166
// --------------------------------------------------------------------------
// Pin definitions are in platform.c as enum + platform_gpio_t arrays,
// matching the SDK's wwd_platform_common.h convention.
// GPIO port/pin mappings are defined there via wifi_control_pins[]
// and wifi_sdio_pins[] arrays.

// --------------------------------------------------------------------------
// WiFi power-up timing
// --------------------------------------------------------------------------

// Delay (ms) after deasserting reset before accessing SDIO bus
#define WICED_WIFI_RESET_DELAY_MS (100)

// Delay (ms) after initial SDIO bus-up before firmware download
#define WICED_WIFI_STARTUP_DELAY_MS (10)

// --------------------------------------------------------------------------
// Platform function declarations
// --------------------------------------------------------------------------

extern void host_platform_init(void);
extern void host_platform_reset_wifi(int reset_asserted);
extern void host_platform_power_wifi(int power_enabled);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef INCLUDED_PLATFORM_H_
