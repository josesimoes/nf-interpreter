//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// WICED configuration defaults for ChibiOS MXCHIP AZ3166 build.
// These defines are normally set by the WICED build system; we provide
// them here for the "bare" CMake compilation of the SDK sources.

#ifndef INCLUDED_WICED_DEFAULTS_H_
#define INCLUDED_WICED_DEFAULTS_H_

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// Target / platform selection
// --------------------------------------------------------------------------

// MCU family
#ifndef PLATFORM_STM32F4XX
#define PLATFORM_STM32F4XX
#endif

// WiFi chip
#ifndef WICED_WIFI_CHIP
#define WICED_WIFI_CHIP BCM43362
#endif

// Bus protocol — SDIO
#ifndef WWD_BUS_PROTOCOL_SDIO
#define WWD_BUS_PROTOCOL_SDIO
#endif

// RTOS selection
#ifndef RTOS_CHIBIOS
#define RTOS_CHIBIOS
#endif

// Network stack selection
#ifndef NETWORK_LwIP
#define NETWORK_LwIP 1
#endif

// --------------------------------------------------------------------------
// WiFi module configuration
// --------------------------------------------------------------------------

// Country code (can be overridden at init time)
#ifndef WICED_DEFAULT_COUNTRY
#define WICED_DEFAULT_COUNTRY WICED_COUNTRY_WORLD_WIDE_XX
#endif

// Wi-Fi firmware is embedded in flash (no filesystem)
#ifndef WICED_RESOURCES_HAVE_INTERNAL
#define WICED_RESOURCES_HAVE_INTERNAL
#endif

// No filesystem resources
#ifndef NO_WICED_API
#define NO_WICED_API
#endif

// Disable TLS — nanoFramework uses its own mbedTLS
#ifndef WICED_DISABLE_TLS
#define WICED_DISABLE_TLS
#endif

// --------------------------------------------------------------------------
// Thread / stack sizes (bytes)
// --------------------------------------------------------------------------

#ifndef WWD_THREAD_STACK_SIZE
#define WWD_THREAD_STACK_SIZE (4096)
#endif

// WWD_THREAD_PRIORITY is NOT defined here because the SDK's wwd_thread.h
// unconditionally defines it as RTOS_HIGHEST_PRIORITY (from wwd_rtos.h).

// --------------------------------------------------------------------------
// Buffer / memory pool configuration
// --------------------------------------------------------------------------

// Maximum number of buffers in the TX pool
#ifndef WICED_WIFI_TX_PACKET_POOL_SIZE
#define WICED_WIFI_TX_PACKET_POOL_SIZE (8)
#endif

// Maximum number of buffers in the RX pool
#ifndef WICED_WIFI_RX_PACKET_POOL_SIZE
#define WICED_WIFI_RX_PACKET_POOL_SIZE (8)
#endif

// --------------------------------------------------------------------------
// SDIO bus configuration
// --------------------------------------------------------------------------

// High-speed SDIO clock (25 MHz for BCM43362)
#ifndef SDIO_CLK_HZ
#define SDIO_CLK_HZ (25000000)
#endif

// SDIO 1-bit or 4-bit mode
#ifndef SDIO_1_BIT
// 4-bit mode by default
#endif

// --------------------------------------------------------------------------
// Debug / assertion control
// --------------------------------------------------------------------------

#ifndef WICED_DISABLE_WATCHDOG
#define WICED_DISABLE_WATCHDOG
#endif

// Map the firmware image symbol name
#ifndef wifi_firmware_image
#define wifi_firmware_image resources_firmware_DIR_43362_DIR_43362A2_bin
#endif

// --------------------------------------------------------------------------
// IOCTL timeout (from SDK's include/wiced_defaults.h, normally pulled via
// the high-level WICED API which we disable with NO_WICED_API)
// --------------------------------------------------------------------------
#ifndef WICED_NEVER_TIMEOUT
#define WICED_NEVER_TIMEOUT (0xFFFFFFFF)
#endif

#ifndef WICED_IOCTL_PACKET_TIMEOUT
#define WICED_IOCTL_PACKET_TIMEOUT (WICED_NEVER_TIMEOUT)
#endif

// Soft-AP DTIM period (from SDK include/wiced_defaults.h:198)
#ifndef WICED_DEFAULT_SOFT_AP_DTIM_PERIOD
#define WICED_DEFAULT_SOFT_AP_DTIM_PERIOD (1)
#endif

// Worker thread priority levels (from SDK include/wiced_defaults.h:155-157)
#ifndef WICED_NETWORK_WORKER_PRIORITY
#define WICED_NETWORK_WORKER_PRIORITY (3)
#endif

#ifndef WICED_DEFAULT_WORKER_PRIORITY
#define WICED_DEFAULT_WORKER_PRIORITY (5)
#endif

#ifndef WICED_DEFAULT_LIBRARY_PRIORITY
#define WICED_DEFAULT_LIBRARY_PRIORITY (5)
#endif

#ifndef WICED_APPLICATION_PRIORITY
#define WICED_APPLICATION_PRIORITY (7)
#endif

// --------------------------------------------------------------------------
// WiFi channel configuration (from SDK include/wiced_defaults.h:258)
// --------------------------------------------------------------------------
#ifndef WICED_WIFI_MAX_CHANNELS
#define WICED_WIFI_MAX_CHANNELS (64)
#endif

// --------------------------------------------------------------------------
// GCC compatibility (from SDK include/wiced_defaults.h:261-263)
// --------------------------------------------------------------------------
#if (defined(__GNUC__) && (__GNUC__ >= 6))
#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef INCLUDED_WICED_DEFAULTS_H_
