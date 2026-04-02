
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

#include <lwip/dhcp.h>
#include <nf_lwipthread.h>
#include <string.h>

// Connect state tracking for WICED WiFi
static volatile bool nf_wiced_connect_in_progress = false;
static volatile int nf_wiced_connect_result = -1; // -1 = pending, 0 = success, 1 = not found, 2 = auth fail, 3 = other

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

#if defined(WWD_BUS_PROTOCOL_SDIO)

int Network_Interface_Start_Connect(int index, const char *ssid, const char *passphrase, int options)
{
    (void)options;
    (void)index;

    wiced_ssid_t wiced_ssid;
    wwd_result_t result;

    // Prepare SSID
    size_t ssidLen = hal_strlen_s(ssid);
    if (ssidLen > 32)
    {
        ssidLen = 32;
    }
    wiced_ssid.length = (uint8_t)ssidLen;
    memcpy(wiced_ssid.value, ssid, ssidLen);

    nf_wiced_connect_in_progress = true;
    nf_wiced_connect_result = -1;

    // Join the network — this is a blocking call
    result = wwd_wifi_join(
        &wiced_ssid,
        WICED_SECURITY_WPA2_AES_PSK,
        (const uint8_t *)passphrase,
        (uint8_t)hal_strlen_s(passphrase),
        NULL,
        WWD_STA_INTERFACE);

    if (result == WWD_SUCCESS)
    {
        // Start DHCP on the WiFi netif
        struct netif *nptr = nf_getNetif();
        netifapi_dhcp_start(nptr);

        nf_wiced_connect_result = 0; // success
    }
    else if (result == WWD_NETWORK_NOT_FOUND || result == WWD_ACCESS_POINT_NOT_FOUND)
    {
        nf_wiced_connect_result = 1; // network not available
    }
    else if (result == WWD_NOT_AUTHENTICATED || result == WWD_EAPOL_KEY_PACKET_M1_TIMEOUT ||
             result == WWD_EAPOL_KEY_PACKET_M3_TIMEOUT || result == WWD_EAPOL_KEY_PACKET_G1_TIMEOUT ||
             result == WWD_EAPOL_KEY_FAILURE || result == WWD_INVALID_KEY)
    {
        nf_wiced_connect_result = 2; // invalid credential
    }
    else
    {
        nf_wiced_connect_result = 3; // unspecified failure
    }

    nf_wiced_connect_in_progress = false;

    return (result == WWD_SUCCESS) ? 0 : -1;
}

int Network_Interface_Connect_Result(int index)
{
    (void)index;

    if (nf_wiced_connect_in_progress)
    {
        return -1; // still pending
    }

    return nf_wiced_connect_result;
}

int Network_Interface_Disconnect(int index)
{
    (void)index;

    // Stop DHCP
    struct netif *nptr = nf_getNetif();
    netifapi_dhcp_stop(nptr);

    // Leave WiFi network
    wwd_wifi_leave(WWD_STA_INTERFACE);

    nf_wiced_connect_result = -1;

    return 0;
}

int Network_Interface_Start_Scan(int index)
{
    (void)index;

    // WiFi scan is not yet implemented for WICED/ChibiOS
    // wwd_wifi_scan() requires a callback-based approach that needs
    // additional infrastructure to integrate with the managed scan API.
    return -1;
}

#else

// Non-WiFi (Ethernet) stubs — these are only called when WiFi is not available
int Network_Interface_Start_Connect(int index, const char *ssid, const char *passphrase, int options)
{
    (void)index;
    (void)ssid;
    (void)passphrase;
    (void)options;
    return SOCK_SOCKET_ERROR;
}

int Network_Interface_Connect_Result(int index)
{
    (void)index;
    return SOCK_SOCKET_ERROR;
}

int Network_Interface_Disconnect(int index)
{
    (void)index;
    return SOCK_SOCKET_ERROR;
}

int Network_Interface_Start_Scan(int index)
{
    (void)index;
    return SOCK_SOCKET_ERROR;
}

#endif // WWD_BUS_PROTOCOL_SDIO
