//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <sys_dev_adc_native_target.h>

// the ADC channels (and respective GPIO pins) available in the managed API
const NF_PAL_ADC_PORT_PIN_CHANNEL c_AdcPortPinConfig[] = {
    // ADC1
    {1, GPIOA, 0, ADC_CHANNEL_IN0},   // Arduino A0
    {1, GPIOC, 0, ADC_CHANNEL_IN10},  // Arduino A3
    {1, GPIOC, 1, ADC_CHANNEL_IN11},  // Arduino A4
    {1, GPIOC, 3, ADC_CHANNEL_IN13},  // Arduino A5
};

const int c_AdcChannelCount = ARRAYSIZE(c_AdcPortPinConfig);
