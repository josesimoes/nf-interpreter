//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <nanoHAL_Types.h>
#include <nanoPAL_BlockStorage.h>

// 8kB blocks
const BlockRange BlockRange1[] = {
    {BlockRange_BLOCKTYPE_BOOTSTRAP, 0, 7}, // 0x08000000 nanoBooter

    ///////////////////////////////////////////////////////////////////////////////////////
    // because this target is using a configuration block need to add the
    // configuration manager files to the CMake and call ConfigurationManager_Initialize()
    // in nanoBooter so the configuration can be managed when in booter mode
    ///////////////////////////////////////////////////////////////////////////////////////
    {BlockRange_BLOCKTYPE_CONFIG, 8, 15}, // 0x08010000 configuration block
    ///////////////////////////////////////////////////////////////////////////////////////

    {BlockRange_BLOCKTYPE_CODE, 16, 255},         // 0x08020000 nanoCLR
    {BlockRange_BLOCKTYPE_DEPLOYMENT, 256, 511} // 0x08200000 deployment (second flash bank)
};

const BlockRegionInfo BlockRegions[] = {
    {
        // STM32H5 flash requires 128bits width programming (flash word)
        (BlockRegionAttribute_ProgramWidthIs128bits),
        0x08000000, // start address for block region
        512,        // total number of blocks in this region
        0x2000,     // total number of bytes per block
        ARRAYSIZE_CONST_EXPR(BlockRange1),
        BlockRange1,
    },
};

const DeviceBlockInfo Device_BlockInfo = {
    (MediaAttribute_SupportsXIP),       // STM32 flash memory is XIP
    16,                                 // UINT32 BytesPerSector (flash word size)
    ARRAYSIZE_CONST_EXPR(BlockRegions), // UINT32 NumRegions;
    (BlockRegionInfo *)BlockRegions,    // const BlockRegionInfo* pRegions;
};

MEMORY_MAPPED_NOR_BLOCK_CONFIG Device_BlockStorageConfig = {
    {
        // BLOCK_CONFIG
        {
            0,     // GPIO_PIN             Pin;
            false, // BOOL                 ActiveState;
        },

        (DeviceBlockInfo *)&Device_BlockInfo, // BlockDeviceinfo
    },

    {
        // CPU_MEMORY_CONFIG
        0,          // UINT8  CPU_MEMORY_CONFIG::ChipSelect;
        true,       // UINT8  CPU_MEMORY_CONFIG::ReadOnly;
        0,          // UINT32 CPU_MEMORY_CONFIG::WaitStates;
        0,          // UINT32 CPU_MEMORY_CONFIG::ReleaseCounts;
        16,         // UINT32 CPU_MEMORY_CONFIG::BitWidth;
        0x08000000, // UINT32 CPU_MEMORY_CONFIG::BaseAddress;
        0x00400000, // UINT32 CPU_MEMORY_CONFIG::SizeInBytes;
        0,          // UINT8  CPU_MEMORY_CONFIG::XREADYEnable
        0,          // UINT8  CPU_MEMORY_CONFIG::ByteSignalsForRead
        0,          // UINT8  CPU_MEMORY_CONFIG::ExternalBufferEnable
    },

    0, // UINT32 ChipProtection;
    0, // UINT32 ManufacturerCode;
    0, // UINT32 DeviceCode;
};

BlockStorageDevice Device_BlockStorage;
