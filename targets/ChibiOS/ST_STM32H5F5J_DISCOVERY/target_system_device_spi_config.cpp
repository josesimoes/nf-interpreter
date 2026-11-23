//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <sys_dev_spi_native_target.h>

//////////
// SPI1 //
//////////
// pin configuration for SPI1 (Arduino connector)
// port for SCK pin is: PG11 (Arduino D13)
// port for MISO pin is: PG9 (Arduino D12)
// port for MOSI pin is: PB5 (Arduino D11)
// GPIO alternate pin function is 5 (see alternate function mapping table in device datasheet)
SPI_CONFIG_PINS(1, GPIOG, 11, GPIOG, 9, GPIOB, 5, 5)
