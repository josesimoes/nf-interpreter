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

#include "Com_SkyworksInc_NanoFramework_Devices_C1.h"
#include "Com_SkyworksInc_NanoFramework_Devices_C1_Com_SkyworksInc_NanoFramework_Devices_C1_C1Bus.h"

using namespace Com_SkyworksInc_NanoFramework_Devices_C1::Com_SkyworksInc_NanoFramework_Devices_C1;


CLR_RT_TypedArray_UINT8 C1Bus::NativeTransmitWrite( uint8_t param0, uint8_t param1, HRESULT &hr )
{

    (void)param0;
    (void)param1;
    (void)hr;
    CLR_RT_TypedArray_UINT8 retValue = 0;

    ////////////////////////////////
    // implementation starts here //
    memcpy((void*)retValue.GetBuffer(), (const void*)0x0122, 

    // implementation ends here   //
    ////////////////////////////////

    return retValue;
}

CLR_RT_TypedArray_UINT8 C1Bus::NativeTransmitRead( uint8_t param0, HRESULT &hr )
{

    (void)param0;
    (void)hr;

    ////////////////////////////////
    // implementation starts here //

    // Define the data array
    CLR_UINT8 data[] = { 0x01, 0x02};

    // Initialize the retur value
    CLR_RT_TypedArray_UINT8 retValue;
    retValue.Set(data, 2);

    // implementation ends here   //
    ////////////////////////////////

    return retValue;
}
