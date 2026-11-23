//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <hal.h>
#include <nanoHAL.h>
#include <nanoHAL_v2.h>
#include <nanoWeak.h>
#include <Target_BlockStorage_STM32FlashDriver.h>

#ifndef UID_BASE
#error "UID_BASE is not defined, it is required to derive a unique default MAC address"
#endif

// Derives a MAC address that is unique for each board from the 96 bit device unique ID.
// The address is locally administered and unicast.
static void GetDefaultMacAddress(uint8_t *macAddress)
{
    const uint8_t *uid = (const uint8_t *)UID_BASE;

    // FNV-1a 64 bit hash of the 12 bytes of the device unique ID
    uint64_t hash = 0xCBF29CE484222325ULL;

    for (uint32_t i = 0; i < 12; i++)
    {
        hash ^= uid[i];
        hash *= 0x100000001B3ULL;
    }

    for (uint32_t i = 0; i < 6; i++)
    {
        macAddress[i] = (uint8_t)(hash >> (8 * i));
    }

    // set the locally administered bit and clear the multicast bit
    macAddress[0] = (macAddress[0] | 0x02) & 0xFE;
}

// Default initialisation for Network interface config blocks
// strong implementation replacing ChibiOS 'weak' one
bool InitialiseNetworkDefaultConfig(HAL_Configuration_NetworkInterface *pconfig, uint32_t configurationIndex)
{
    (void)configurationIndex;

    // zero memory
    memset(pconfig, 0, sizeof(HAL_Configuration_NetworkInterface));

    // make sure the config block marker is set
    memcpy(pconfig->Marker, c_MARKER_CONFIGURATION_NETWORK_V1, sizeof(c_MARKER_CONFIGURATION_NETWORK_V1));

    pconfig->InterfaceType = NetworkInterfaceType_Ethernet;
    pconfig->StartupAddressMode = AddressMode_DHCP;
    pconfig->AutomaticDNS = 1;
    pconfig->SpecificConfigId = UINT32_MAX;

    // set a MAC address that is unique for each board (only used when there is no stored network configuration)
    GetDefaultMacAddress(pconfig->MacAddress);

    return true;
}
