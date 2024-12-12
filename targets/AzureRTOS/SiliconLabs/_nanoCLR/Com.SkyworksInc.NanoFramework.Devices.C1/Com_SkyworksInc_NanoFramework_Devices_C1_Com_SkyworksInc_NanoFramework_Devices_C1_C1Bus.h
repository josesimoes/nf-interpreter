//-----------------------------------------------------------------------------
//
//                   ** WARNING! ** 
//    This file was generated automatically by a tool.
//    Re-running the tool will overwrite this file.
//    You should copy this file to a custom location
//    before adding any customization in the copy to
//    prevent loss of your changes when the tool is
//    re-run.
//
//-----------------------------------------------------------------------------

#ifndef COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_C1BUS_H
#define COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_C1BUS_H

// #include "em_device.h"

// #define PB10_BIT            (1 << 10)
// #define GPIO_READ()         ((GPIO->P[1].DIN & PB10_BIT) ? 1 : 0)

// // GPIO_SIGNAL(0) = LOW, GPIO_SIGNAL(1) = HIGH
// #define GPIO_SIGNAL(value)  (value ? (GPIO->P[1].DOUT |= (1 << 10)) : (GPIO->P[1].DOUT &= ~(1 << 10)))

namespace Com_SkyworksInc_NanoFramework_Devices_C1
{
    namespace Com_SkyworksInc_NanoFramework_Devices_C1
    {
        struct C1Bus
        {
            // Helper Functions to access fields of managed object
            // Declaration of stubs. These functions are implemented by Interop code developers

            static void NativeTransmitWriteWithAddress( uint8_t param0, uint8_t param1, CLR_RT_TypedArray_UINT8 param2, HRESULT &hr );

            static void NativeTransmitReadWithAddress( uint8_t param0, CLR_RT_TypedArray_UINT8 param1, HRESULT &hr );

            static void NativeTransmitRead( CLR_RT_TypedArray_UINT8 param0, HRESULT &hr );

            static void NativeTransmitWrite( uint8_t param0, CLR_RT_TypedArray_UINT8 param1, HRESULT &hr );

            static void NativeTransmitWriteAddress( uint8_t param0, CLR_RT_TypedArray_UINT8 param1, HRESULT &hr );

            static void NativeTransmitReadAddress( CLR_RT_TypedArray_UINT8 param0, HRESULT &hr );

        };
    }
}

#endif // COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_COM_SKYWORKSINC_NANOFRAMEWORK_DEVICES_C1_C1BUS_H
