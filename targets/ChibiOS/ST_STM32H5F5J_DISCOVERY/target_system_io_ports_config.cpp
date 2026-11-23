//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include "target_system_io_ports_config.h"
#include <sys_io_ser_native_target.h>

///////////
// UART7 //
///////////

// pin configuration for UART7 (Arduino connector)
// port for TX pin is: GPIOE
// port for RX pin is: GPIOE
// TX pin: is GPIOE_8 (Arduino D1)
// RX pin: is GPIOE_7 (Arduino D0)
// GPIO alternate pin function is 7 (see alternate function mapping table in device datasheet)
UART_CONFIG_PINS(7, GPIOE, GPIOE, 8, 7, 7)

// initialization for UART7
UART_INIT(7)

// un-initialization for UART7
UART_UNINIT(7)
