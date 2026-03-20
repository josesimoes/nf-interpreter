//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// Minimal resource type definitions for WICED firmware blob access.
// The full WICED SDK defines these in wiced_resource.h; this provides
// just enough for the embedded firmware blob in 43362A2_bin.c.

#ifndef INCLUDED_WICED_RESOURCE_H_
#define INCLUDED_WICED_RESOURCE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Resource location type
typedef enum
{
    RESOURCE_IN_MEMORY,
    RESOURCE_IN_FILESYSTEM,
    RESOURCE_IN_EXTERNAL_STORAGE
} resource_location_t;

// Resource handle — describes where to find a data blob
typedef struct
{
    resource_location_t location;
    uint32_t size;
    union
    {
        struct { const char *data; } mem;  // RESOURCE_IN_MEMORY: pointer to data
        const char *fs;                     // RESOURCE_IN_FILESYSTEM: path string
    } val;
} resource_hnd_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef INCLUDED_WICED_RESOURCE_H_
