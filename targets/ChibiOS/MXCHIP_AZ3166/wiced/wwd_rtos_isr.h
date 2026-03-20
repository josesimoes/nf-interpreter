//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// ChibiOS ISR helpers for WICED WWD.
// WICED requires macros that wrap ISR entry/exit with RTOS-specific logic.
// On ChibiOS this maps to CH_IRQ_PROLOGUE / CH_IRQ_EPILOGUE.

#ifndef INCLUDED_WWD_RTOS_ISR_H_
#define INCLUDED_WWD_RTOS_ISR_H_

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Enter an ISR — save context for ChibiOS
#define WWD_RTOS_DEFINE_ISR(function) CH_IRQ_HANDLER(function)

// Map an ISR handler to a hardware vector name.
// On ChibiOS, ISR vectors are handled by the HAL layer, so this is a no-op.
// The SDK uses: WWD_RTOS_MAP_ISR(handler, vector_name)
#define WWD_RTOS_MAP_ISR(function, vector) /* ChibiOS handles ISR mapping */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef INCLUDED_WWD_RTOS_ISR_H_
