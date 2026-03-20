/*
 * Copyright (c) .NET Foundation and Contributors
 * See LICENSE file in the project root for full license information.
 *
 * Minimal resource provider for WICED WWD on ChibiOS.
 * Uses WWD_DIRECT_RESOURCES mode — firmware and NVRAM are embedded in flash.
 */

#include <string.h>
#include <stdint.h>
#include "wwd_constants.h"
#include "wiced_resource.h"
#include "wifi_nvram_image.h"
#include "platform/wwd_resource_interface.h"

extern const resource_hnd_t wifi_firmware_image;

static uint32_t resource_get_size(const resource_hnd_t *resource)
{
    return resource->size;
}

wwd_result_t host_platform_resource_size(wwd_resource_t resource, uint32_t *size_out)
{
    if (resource == WWD_RESOURCE_WLAN_FIRMWARE)
    {
        *size_out = (uint32_t)resource_get_size(&wifi_firmware_image);
    }
    else
    {
        *size_out = sizeof(wifi_nvram_image);
    }
    return WWD_SUCCESS;
}

wwd_result_t host_platform_resource_read_direct(wwd_resource_t resource, const void **ptr_out)
{
    if (resource == WWD_RESOURCE_WLAN_FIRMWARE)
    {
        *ptr_out = wifi_firmware_image.val.mem.data;
    }
    else
    {
        *ptr_out = wifi_nvram_image;
    }
    return WWD_SUCCESS;
}
