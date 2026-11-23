//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

namespace nanoFramework.Targets.STM32H5F5J_DISCOVERY
{
    /// <summary>
    /// Enumeration of ADC channels available in STM32H5F5J_DISCOVERY
    /// </summary>
    public static class AdcChannels
    {
        /// <summary>
        /// Channel 0, exposed on Arduino A0, connected to PA0 (ADC1 - INP0)
        /// </summary>
        public const int Channel_0 = 0;

        /// <summary>
        /// Channel 1, exposed on Arduino A3, connected to PC0 (ADC1 - INP10)
        /// </summary>
        public const int Channel_1 = 1;

        /// <summary>
        /// Channel 2, exposed on Arduino A4, connected to PC1 (ADC1 - INP11)
        /// </summary>
        public const int Channel_2 = 2;

        /// <summary>
        /// Channel 3, exposed on Arduino A5, connected to PC3 (ADC1 - INP13)
        /// </summary>
        public const int Channel_3 = 3;
    }
}
