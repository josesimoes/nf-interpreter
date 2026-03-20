
//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <nanoHAL.h>
#include <lwip/netifapi.h>

#if defined(WIFI_DRIVER_ISM43362) || defined(WWD_BUS_PROTOCOL_SDIO)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "wwd_management.h"
#include "wwd_wifi.h"
#pragma GCC diagnostic pop
#endif

extern "C" struct netif *nf_getNetif();

//
// Works with the Target_NetworkConfig to map the Network_Interface_XXXXX calls to the correct driver

bool Network_Interface_Bind(int index)
{
    (void)index;

    return true;
}

int Network_Interface_Open(int index)
{
    HAL_Configuration_NetworkInterface networkConfiguration;

    // load network interface configuration from storage
    if (!ConfigurationManager_GetConfigurationBlock(
            (void *)&networkConfiguration,
            DeviceConfigurationOption_Network,
            index))
    {
        // failed to load configuration
        // FIXME output error?
        return SOCK_SOCKET_ERROR;
    }
    _ASSERTE(networkConfiguration.StartupAddressMode > 0);

    switch (index)
    {
        case 0:
        {
            // Open the network interface and set its config
            // TODO / FIXME

            // Return index to NetIF in its linked list, return 0 (probably right if only interface)
            // This used by Network stack to hook in to status/address changes for events to users

            // For now get the Netif number form original Chibios binding code
            struct netif *nptr = nf_getNetif();
            return nptr->num;
        }
        break;
    }
    return SOCK_SOCKET_ERROR;
}

bool Network_Interface_Close(int index)
{
    switch (index)
    {
        case 0:
#if HAL_USE_MAC
            macStop(&ETHD1);
#elif defined(WWD_BUS_PROTOCOL_SDIO)
            wwd_wifi_leave(WWD_STA_INTERFACE);
            wwd_management_wifi_off();
#endif
            return true;
    }
    return false;
}
