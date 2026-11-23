//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <sys_dev_i2c_native_target.h>

//////////
// I2C3 //
//////////
// pin configuration for I2C3 (Arduino connector)
// port for SCL pin is: GPIOH
// port for SDA pin is: GPIOH
// SCL pin: is GPIOH_7 (Arduino D15)
// SDA pin: is GPIOH_8 (Arduino D14)
// GPIO alternate pin function is 4 (see alternate function mapping table in device datasheet)
I2C_CONFIG_PINS(3, GPIOH, GPIOH, 7, 8, 4)
