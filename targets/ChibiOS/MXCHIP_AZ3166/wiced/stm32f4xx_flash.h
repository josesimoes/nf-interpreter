//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// Stub for stm32f4xx_flash.h — prevents WICED SDK's bundled STM32 Standard
// Peripheral Library flash header from conflicting with ChibiOS's hal_flash.h
// (both define FLASH_ERROR_PROGRAM enum constant).
//
// No WICED WiFi code we compile needs STM32 flash operations.

#ifndef __STM32F4xx_FLASH_H
#define __STM32F4xx_FLASH_H

// Intentionally empty — the WICED SDIO/WiFi drivers do not use flash.

#endif // __STM32F4xx_FLASH_H
