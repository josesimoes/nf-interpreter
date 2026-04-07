//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// AZ3166 platform implementation for WICED WWD.
// Handles BCM43362 power-on/off, reset sequencing, and provides
// platform_gpio_t pin arrays required by wwd_SDIO.c and platform_gpio.c.

#include "platform.h"
#include "wiced_defaults.h"
#include "wwd_rtos.h"
#include "wwd_constants.h"
#include "platform/wwd_resource_interface.h"

// STM32F412 register definitions (ChibiOS CMSIS v5 — compatible addresses)
#include "stm32f4xx.h"

#include <string.h>
#include <stdint.h>

// --------------------------------------------------------------------------
// platform_gpio_t — matches WICED SDK's definition from
// platform_mcu_peripheral.h: { GPIO_TypeDef *port, uint8_t pin_number }
// We define it here to avoid pulling in the full StdPeriph header chain.
// --------------------------------------------------------------------------
typedef struct
{
    GPIO_TypeDef *port;
    uint8_t       pin_number;
} platform_gpio_t;

// --------------------------------------------------------------------------
// Pin index enums — must match wwd_platform_common.h
// --------------------------------------------------------------------------
enum
{
    WWD_PIN_POWER       = 0,
    WWD_PIN_RESET       = 1,
    WWD_PIN_32K_CLK     = 2,
    WWD_PIN_BOOTSTRAP_0 = 3,
    WWD_PIN_BOOTSTRAP_1 = 4,
    WWD_PIN_CONTROL_MAX = 5,
};

enum
{
    WWD_PIN_SDIO_OOB_IRQ = 0,
    WWD_PIN_SDIO_CLK     = 1,
    WWD_PIN_SDIO_CMD     = 2,
    WWD_PIN_SDIO_D0      = 3,
    WWD_PIN_SDIO_D1      = 4,
    WWD_PIN_SDIO_D2      = 5,
    WWD_PIN_SDIO_D3      = 6,
    WWD_PIN_SDIO_MAX     = 7,
};

// --------------------------------------------------------------------------
// Pin arrays — required by wwd_SDIO.c, platform_gpio.c, and
// wwd_platform_separate_mcu.c (which we've replaced)
// --------------------------------------------------------------------------

const platform_gpio_t wifi_control_pins[] =
{
    [WWD_PIN_POWER]       = { GPIOA,  9 },  // PA9  — WiFi power/reset
    [WWD_PIN_RESET]       = { GPIOA,  9 },  // PA9  — same pin on AZ3166
    [WWD_PIN_32K_CLK]     = { GPIOA,  8 },  // PA8  — 32 kHz clock output (MCO1)
    [WWD_PIN_BOOTSTRAP_0] = { GPIOB,  0 },  // PB0  — bootstrap for SDIO mode
    [WWD_PIN_BOOTSTRAP_1] = { GPIOB,  1 },  // PB1  — bootstrap for SDIO mode
};

const platform_gpio_t wifi_sdio_pins[] =
{
    [WWD_PIN_SDIO_OOB_IRQ] = { GPIOC,  0 },  // PC0  — OOB interrupt
    [WWD_PIN_SDIO_CLK]     = { GPIOC, 12 },  // PC12 — SDIO_CLK, AF12
    [WWD_PIN_SDIO_CMD]     = { GPIOD,  2 },  // PD2  — SDIO_CMD, AF12
    [WWD_PIN_SDIO_D0]      = { GPIOC,  8 },  // PC8  — SDIO_D0, AF12
    [WWD_PIN_SDIO_D1]      = { GPIOC,  9 },  // PC9  — SDIO_D1, AF12
    [WWD_PIN_SDIO_D2]      = { GPIOC, 10 },  // PC10 — SDIO_D2, AF12
    [WWD_PIN_SDIO_D3]      = { GPIOB,  5 },  // PB5  — SDIO_D3, AF12
};

// --------------------------------------------------------------------------
// Helper: GPIO pin setup via direct register access
// --------------------------------------------------------------------------

static void gpio_set_pin_output(GPIO_TypeDef *port, uint32_t pin)
{
    // Set the pin to general-purpose output, push-pull, medium speed, no pull
    uint32_t pos = pin * 2;

    // Mode: output (01)
    port->MODER = (port->MODER & ~(3UL << pos)) | (1UL << pos);
    // Output type: push-pull (0)
    port->OTYPER &= ~(1UL << pin);
    // Speed: medium (01)
    port->OSPEEDR = (port->OSPEEDR & ~(3UL << pos)) | (1UL << pos);
    // No pull-up/pull-down (00)
    port->PUPDR &= ~(3UL << pos);
}

static void gpio_set_pin_high(GPIO_TypeDef *port, uint32_t pin)
{
    port->BSRR = (1UL << pin);
}

static void gpio_set_pin_low(GPIO_TypeDef *port, uint32_t pin)
{
    port->BSRR = (1UL << (pin + 16));
}

// --------------------------------------------------------------------------
// Platform initialization
// --------------------------------------------------------------------------

wwd_result_t host_platform_init(void)
{
    // Enable GPIO clocks for ports A, B, C, D (if not already enabled)
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
                      RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN);

    // Configure control pins as outputs
    gpio_set_pin_output(wifi_control_pins[WWD_PIN_RESET].port,
                        wifi_control_pins[WWD_PIN_RESET].pin_number);
    gpio_set_pin_output(wifi_control_pins[WWD_PIN_BOOTSTRAP_0].port,
                        wifi_control_pins[WWD_PIN_BOOTSTRAP_0].pin_number);
    gpio_set_pin_output(wifi_control_pins[WWD_PIN_BOOTSTRAP_1].port,
                        wifi_control_pins[WWD_PIN_BOOTSTRAP_1].pin_number);

    // Set bootstrap pins for SDIO mode:
    //   BOOTSTRAP_0 = LOW  → SDIO mode
    //   BOOTSTRAP_1 = LOW  → SDIO mode
    gpio_set_pin_low(wifi_control_pins[WWD_PIN_BOOTSTRAP_0].port,
                     wifi_control_pins[WWD_PIN_BOOTSTRAP_0].pin_number);
    gpio_set_pin_low(wifi_control_pins[WWD_PIN_BOOTSTRAP_1].port,
                     wifi_control_pins[WWD_PIN_BOOTSTRAP_1].pin_number);

    // Assert reset (active low → drive LOW to hold in reset)
    gpio_set_pin_low(wifi_control_pins[WWD_PIN_RESET].port,
                     wifi_control_pins[WWD_PIN_RESET].pin_number);

    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Reset sequencing
// --------------------------------------------------------------------------

wwd_result_t host_platform_reset_wifi(int reset_asserted)
{
    if (reset_asserted)
    {
        gpio_set_pin_low(wifi_control_pins[WWD_PIN_RESET].port,
                         wifi_control_pins[WWD_PIN_RESET].pin_number);
    }
    else
    {
        gpio_set_pin_high(wifi_control_pins[WWD_PIN_RESET].port,
                          wifi_control_pins[WWD_PIN_RESET].pin_number);
    }

    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Power control
// --------------------------------------------------------------------------

wwd_result_t host_platform_power_wifi(int power_enabled)
{
    if (power_enabled)
    {
        // Power on: release reset and wait for module startup
        host_platform_reset_wifi(0);
        host_rtos_delay_milliseconds(WICED_WIFI_RESET_DELAY_MS);
    }
    else
    {
        // Power off: hold in reset
        host_platform_reset_wifi(1);
    }

    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Power save stubs — wwd_SDIO.c and platform_gpio.c reference these
// (platform_mcu_powersave.c was removed due to deep StdPeriph deps)
// --------------------------------------------------------------------------

void platform_mcu_powersave_disable(void) { }
void platform_mcu_powersave_enable(void) { }
void platform_mcu_powersave_exit_notify(void) { }
void host_platform_deinit(void) { }
void host_platform_deinit_wlan_powersave_clock(void) { }

// --------------------------------------------------------------------------
// Resource read indirect stub — wwd_bus_common.c references this but
// we use WWD_DIRECT_RESOURCES mode, so it should never be called.
// --------------------------------------------------------------------------

wwd_result_t host_platform_resource_read_indirect(wwd_resource_t resource,
                                                   uint32_t offset,
                                                   void *buffer,
                                                   uint32_t buffer_size,
                                                   uint32_t *size_out)
{
    (void)resource; (void)offset; (void)buffer; (void)buffer_size;
    if (size_out) *size_out = 0;
    return WWD_DOES_NOT_EXIST;
}

// --------------------------------------------------------------------------
// wiced_get_default_ready_interface — referenced by wwd_wifi.c
// --------------------------------------------------------------------------

typedef enum
{
    WICED_STA_INTERFACE  = 0,
    WICED_AP_INTERFACE   = 1,
    WICED_P2P_INTERFACE  = 2,
    WICED_INTERFACE_MAX  = 3,
} wiced_interface_t;

typedef uint32_t wiced_result_t;
#define WICED_SUCCESS  0
#define WICED_ERROR    1

wiced_result_t wiced_get_default_ready_interface(wiced_interface_t *interface)
{
    /* Default to STA interface */
    if (interface) *interface = WICED_STA_INTERFACE;
    return WICED_SUCCESS;
}

// --------------------------------------------------------------------------
// strnstrn — length-limited strstr, referenced by wwd_wifi.c
// --------------------------------------------------------------------------

char *strnstrn(const char *s, uint16_t s_len, const char *substr, uint16_t substr_len)
{
    uint16_t i;
    if (substr_len == 0) return (char *)s;
    if (s_len < substr_len) return NULL;

    for (i = 0; i <= s_len - substr_len; i++)
    {
        if (memcmp(s + i, substr, substr_len) == 0)
        {
            return (char *)(s + i);
        }
    }
    return NULL;
}
