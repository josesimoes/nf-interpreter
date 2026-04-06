
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

// Scan state tracking for WICED WiFi
#define NF_WICED_SCAN_MAX_RESULTS 20
#define WIFI_EVENT_TYPE_SCAN_COMPLETE 1

static volatile bool nf_wiced_scan_in_progress = false;
static wiced_scan_result_t nf_wiced_scan_results[NF_WICED_SCAN_MAX_RESULTS];
static volatile uint16_t nf_wiced_scan_count = 0;
// WICED writes each AP record into this scratch struct; the callback copies to nf_wiced_scan_results[]
static wiced_scan_result_t nf_wiced_scan_result_scratch;
// Pointer passed to wwd_wifi_scan (wiced_scan_result_t**); always points at the scratch struct
static wiced_scan_result_t *nfWicedScanResult = &nf_wiced_scan_result_scratch;

extern void PostManagedEvent(uint8_t category, uint8_t subCategory, uint16_t data1, uint32_t data2);

static void nf_wiced_scan_callback(wiced_scan_result_t **scanResult, void *user_data, wiced_scan_status_t status)
{
    (void)user_data;

    if (scanResult == NULL || status != WICED_SCAN_INCOMPLETE)
    {
        // Scan complete (or aborted) — notify managed code
        nf_wiced_scan_in_progress = false;
        PostManagedEvent(50 /*EVENT_WIFI*/, WIFI_EVENT_TYPE_SCAN_COMPLETE, 0, 0);
        return;
    }

    // Copy the result WICED wrote into the scratch buffer into our permanent array
    if (nf_wiced_scan_count < NF_WICED_SCAN_MAX_RESULTS && *scanResult != NULL)
    {
        memcpy(&nf_wiced_scan_results[nf_wiced_scan_count], *scanResult, sizeof(wiced_scan_result_t));
        nf_wiced_scan_count++;
    }
    // *scanResult is left unchanged — WICED reuses the same scratch buffer for the next AP
}

// Serialize scan results into the ScanRecord byte format expected by managed code.
//   Layout: uint16_t recordCount, then packed ScanRecord structs.
//   ScanRecord: bssid[6], ssid[33], rssi (uint8_t, cast from int8_t), authMode, cypherType.
//
// Returns the number of bytes written to buf, or the required buffer size when buf is NULL.
int Network_Interface_WICED_SerializeScanResults(uint8_t *buf)
{
    uint16_t count = nf_wiced_scan_count;

    // Each record: 6 (bssid) + 33 (ssid) + 1 (rssi) + 1 (authMode) + 1 (cypherType) = 42 bytes
    // Plus 2-byte record count header
    int totalLen = (int)(sizeof(uint16_t) + count * 42u);

    if (buf == NULL)
    {
        return totalLen;
    }

    // Write record count (little-endian)
    buf[0] = (uint8_t)(count & 0xFF);
    buf[1] = (uint8_t)((count >> 8) & 0xFF);
    uint8_t *p = buf + 2;

    for (uint16_t i = 0; i < count; i++)
    {
        const wiced_scan_result_t *src = &nf_wiced_scan_results[i];

        // BSSID (6 bytes)
        memcpy(p, src->BSSID.octet, 6);
        p += 6;

        // SSID (33 bytes, null-terminated)
        uint8_t ssidLen = src->SSID.length;
        if (ssidLen > 32)
            ssidLen = 32;
        memcpy(p, src->SSID.value, ssidLen);
        p[ssidLen] = '\0';
        p += 33;

        // RSSI: truncate int16_t to int8_t (RSSI typically -100..0 dBm)
        *p++ = (uint8_t)(int8_t)(src->signal_strength);

        // authMode: map WICED security flags to nanoFramework AuthenticationType
        uint8_t authMode;
        if (src->security == WICED_SECURITY_OPEN)
            authMode = 4; // AuthenticationType_Open
        else if (src->security & WEP_ENABLED)
            authMode = 6; // AuthenticationType_WEP
        else if ((src->security & WPA2_SECURITY) && (src->security & AES_ENABLED))
            authMode = 8; // AuthenticationType_WPA2
        else if (src->security & WPA_SECURITY)
            authMode = 7; // AuthenticationType_WPA
        else
            authMode = 4;
        *p++ = authMode;

        // cypherType: map WICED security flags to nanoFramework EncryptionType
        uint8_t cipherType;
        if (src->security == WICED_SECURITY_OPEN)
            cipherType = 0; // EncryptionType_None
        else if (src->security & WEP_ENABLED)
            cipherType = 1; // EncryptionType_WEP
        else if ((src->security & WPA2_SECURITY) && (src->security & AES_ENABLED))
            cipherType = 5; // EncryptionType_WPA2_PSK
        else if (src->security & WPA_SECURITY)
            cipherType = 4; // EncryptionType_WPA_PSK
        else
            cipherType = 0;
        *p++ = cipherType;
    }

    return totalLen;
}

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
    HAL_Configuration_NetworkInterface networkConfiguration;

    if (!ConfigurationManager_GetConfigurationBlock(
            (void *)&networkConfiguration,
            DeviceConfigurationOption_Network,
            index))
    {
        return 10; // StartScanOutcome_FailedToGetConfiguration
    }

    if (networkConfiguration.InterfaceType != NetworkInterfaceType_Wireless80211)
    {
        return 20; // StartScanOutcome_WrongInterfaceType
    }

    // Reset scan state
    nf_wiced_scan_count = 0;
    nfWicedScanResult = &nf_wiced_scan_result_scratch; // ensure valid before passing
    nf_wiced_scan_in_progress = true;

    wwd_result_t result = wwd_wifi_scan(
        WICED_SCAN_TYPE_PASSIVE,
        WICED_BSS_TYPE_ANY,
        NULL, // all SSIDs
        NULL, // all BSSIDs
        NULL, // all channels
        NULL, // no extended params
        nf_wiced_scan_callback,
        &nfWicedScanResult, // wiced_scan_result_t** — WICED fills *ptr each call
        NULL,
        WWD_STA_INTERFACE);

    if (result != WWD_SUCCESS)
    {
        nf_wiced_scan_in_progress = false;
        return -1; // StartScanOutcome_Fail
    }

    return 0; // StartScanOutcome_Success
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
