#
# Copyright (c) .NET Foundation and Contributors
# See LICENSE file in the project root for full license information.
#

# WICED SDK build integration for ChibiOS / MXCHIP AZ3166.
#
# This CMakeLists.txt compiles the WICED SDK v6.6.1 source tree (fetched
# via FetchContent) into a static library linked into nanoCLR. It replaces
# the pre-compiled libwiced_sdk_bin.a that was built against ThreadX/NetX.
#
# Usage: included from the target-level CMakeLists.txt when
#        WIFI_DRIVER == "WICED_BCM43362".
#
# Requires:
#   WICED_SDK_PATH — set by the FetchContent block in targets/ChibiOS/CMakeLists.txt
#   lwIP_SOURCE_DIR — the lwIP source directory from nf_add_lwip_library()

# Guard — only build when the driver is selected
if(NOT "${WIFI_DRIVER}" STREQUAL "WICED_BCM43362")
    return()
endif()

if(NOT DEFINED WICED_SDK_PATH)
    message(FATAL_ERROR "WICED_SDK_PATH is not set. Ensure FetchContent for WICED SDK ran successfully.")
endif()

# --------------------------------------------------------------------------
# Source files from the WICED SDK tree
# All paths relative to ${WICED_SDK_PATH}
# --------------------------------------------------------------------------

set(WICED_SDK_SOURCES

    # ----- WiFi core (WWD internal) -----
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_management.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_wifi.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_wifi_chip_common.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_wifi_sleep.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_sdpcm.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_thread.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_internal.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_eapol.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_debug.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/wwd_ap_common.c

    # ----- Bus protocol: SDIO -----
    ${WICED_SDK_PATH}/WICED/WWD/internal/bus_protocols/wwd_bus_common.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/bus_protocols/SDIO/wwd_bus_protocol.c

    # ----- Chip-specific: BCM43362 -----
    ${WICED_SDK_PATH}/WICED/WWD/internal/chips/43362/wwd_ap.c
    ${WICED_SDK_PATH}/WICED/WWD/internal/chips/43362/wwd_chip_specific_functions.c

    # ----- TLV utility (used by wwd_wifi.c) -----
    ${WICED_SDK_PATH}/libraries/utilities/TLV/tlv.c

    # ----- STM32F4xx SDIO platform driver -----
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/WWD/wwd_SDIO.c
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/WWD/wwd_platform.c
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/platform_gpio.c
    # platform_mcu_powersave.c excluded — deep StdPeriph RCC/PWR dependency,
    # stubs for powersave functions provided in platform.c if needed.

    # ----- STM32 Standard Peripheral Library (bundled with WICED SDK) -----
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_sdio.c
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_gpio.c
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_rcc.c
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_exti.c
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries/src/stm32f4xx_syscfg.c

    # ----- Platform abstraction layer -----
    ${WICED_SDK_PATH}/WICED/platform/ARM_CM4/host_cm4.c
    # platform_resource.c REMOVED — deep WICED infrastructure deps (wicedfs, DCT)
    # wwd_platform_separate_mcu.c REMOVED — our platform.c provides host_platform_init/reset/power
    # SDK wwd_resources.c REMOVED — replaced by port version below

    # ----- lwIP network integration -----
    # SDK wwd_buffer.c REMOVED — uses WICED custom lwIP pool types (PBUF_POOL_TX/RX)
    # SDK wwd_network.c REMOVED — uses WICED custom lwIP extensions

    # ----- ChibiOS RTOS port (nanoFramework — NEW) -----
    ${CMAKE_CURRENT_SOURCE_DIR}/wiced/wwd_rtos.c

    # ----- AZ3166 platform definition (nanoFramework — NEW) -----
    ${CMAKE_CURRENT_SOURCE_DIR}/wiced/platform.c

    # ----- Port replacements for deeply-incompatible SDK files -----
    ${CMAKE_CURRENT_SOURCE_DIR}/wiced/wwd_resources.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wiced/wwd_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wiced/wwd_network.c

    # ----- Firmware blob -----
    ${CMAKE_CURRENT_SOURCE_DIR}/common/43362A2_bin.c
)

# --------------------------------------------------------------------------
# Include directories
# --------------------------------------------------------------------------

set(WICED_SDK_INCLUDE_DIRS

    # Our ChibiOS RTOS port and platform headers (searched FIRST)
    ${CMAKE_CURRENT_SOURCE_DIR}/wiced

    # Target-specific headers (resources.h, wiced_sdk.h, etc.)
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}

    # ----- WICED SDK headers -----

    # WWD API
    ${WICED_SDK_PATH}/WICED/WWD
    ${WICED_SDK_PATH}/WICED/WWD/include
    ${WICED_SDK_PATH}/WICED/WWD/include/network
    ${WICED_SDK_PATH}/WICED/WWD/include/RTOS
    ${WICED_SDK_PATH}/WICED/WWD/internal
    ${WICED_SDK_PATH}/WICED/WWD/internal/bus_protocols/SDIO
    ${WICED_SDK_PATH}/WICED/WWD/internal/chips/43362

    # Platform: STM32F4xx
    ${WICED_SDK_PATH}/WICED/platform
    ${WICED_SDK_PATH}/WICED/platform/include
    ${WICED_SDK_PATH}/WICED/platform/MCU
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/WWD
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries
    ${WICED_SDK_PATH}/WICED/platform/MCU/STM32F4xx/peripherals/libraries/inc
    ${WICED_SDK_PATH}/WICED/platform/ARM_CM4

    # Network: lwIP integration
    ${WICED_SDK_PATH}/WICED/network/LwIP/WWD

    # WICED top-level includes
    ${WICED_SDK_PATH}/include
    ${WICED_SDK_PATH}/libraries/utilities/linked_list

    # WICED internal
    ${WICED_SDK_PATH}/WICED/internal

    # TLV utility (for tlv.h)
    ${WICED_SDK_PATH}/libraries/utilities/TLV

    # GCC toolchain definitions (for platform_toolchain.h)
    ${WICED_SDK_PATH}/WICED/platform/GCC

    # Security BESL structures (besl_structures.h, needed by wiced_result.h)
    ${WICED_SDK_PATH}/WICED/security/BESL/include

    # Ring buffer utility (ring_buffer.h, needed by platform_mcu_peripheral.h)
    ${WICED_SDK_PATH}/libraries/utilities/ring_buffer

    # WiFi utilities (wifi_utils.h, needed by wwd_internal.c)
    ${WICED_SDK_PATH}/libraries/utilities/wifi

    # WICED network integration for lwIP (wiced_network.h, needed by wiced_tcpip.h)
    ${WICED_SDK_PATH}/WICED/network/LwIP/WICED

    # BESL host headers (tls_types.h, dtls_types.h, needed by wiced_network.h)
    ${WICED_SDK_PATH}/WICED/security/BESL/host/WICED
)

# --------------------------------------------------------------------------
# Compile definitions
# --------------------------------------------------------------------------

set(WICED_SDK_COMPILE_DEFINITIONS

    # Disable WICED TLS — nanoFramework uses its own mbedTLS
    -DWICED_DISABLE_TLS

    # Network stack selection
    -DNETWORK_LwIP=1

    # Firmware image symbol name
    -Dwifi_firmware_image=resources_firmware_DIR_43362_DIR_43362A2_bin

    # MCU — STM32F412xG is the WICED/StdPeriph convention;
    #        STM32F412Rx is the ARM CMSIS / ChibiOS convention.
    #        Both are needed because ChibiOS CMSIS headers are found first
    #        in the include path and use the Rx naming to select device.
    -DSTM32F412xG
    -DSTM32F412Rx

    # Bus protocol
    -DWWD_BUS_PROTOCOL_SDIO

    # No WICED high-level API (we use WWD directly)
    -DNO_WICED_API

    # WiFi chip
    -DWICED_WIFI_CHIP=BCM43362

    # ChibiOS RTOS
    -DRTOS_CHIBIOS

    # WiFi needs multiple netif support (default lwipopts.h sets LWIP_SINGLE_NETIF=1)
    -DLWIP_SINGLE_NETIF=0
)

# --------------------------------------------------------------------------
# Export variables for the parent CMakeLists.txt
# --------------------------------------------------------------------------

set(WICED_SDK_SOURCES ${WICED_SDK_SOURCES} CACHE INTERNAL "WICED SDK source files")
set(WICED_SDK_INCLUDE_DIRS ${WICED_SDK_INCLUDE_DIRS} CACHE INTERNAL "WICED SDK include directories")
set(WICED_SDK_COMPILE_DEFINITIONS ${WICED_SDK_COMPILE_DEFINITIONS} CACHE INTERNAL "WICED SDK compile definitions")
