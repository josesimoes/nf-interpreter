//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include "target_nf_dev_onewire_config.h"
#include <nf_dev_onewire_target.h>

///////////
// UART5 //
///////////

// pin configuration for UART5
// port for TX pin is: GPIOC (Arduino D5)
// TX pin: is GPIOC_12
// GPIO alternate pin function is 8
UART_CONFIG_PINS(5, GPIOC, 12, 8)

// buffers
uint8_t Uart5_TxBuffer[UART5_TX_SIZE];
uint8_t Uart5_RxBuffer[UART5_RX_SIZE];
