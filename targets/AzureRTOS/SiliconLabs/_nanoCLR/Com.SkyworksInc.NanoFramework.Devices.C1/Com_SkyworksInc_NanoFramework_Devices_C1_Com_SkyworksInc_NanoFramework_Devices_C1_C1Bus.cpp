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


void C1Bus::NativeTransmitWrite( uint8_t param0, uint8_t param1, CLR_RT_TypedArray_UINT8 param2, HRESULT &hr )
{

    (void)param0;
    (void)param1;
    (void)param2;
    (void)hr;


    ////////////////////////////////
    // implementation starts here //

    // param0 is register address
    // param1 is write value for address
    // param2 is response array


    // implementation ends here   //
    ////////////////////////////////


}

void C1Bus::NativeTransmitRead( uint8_t param0, CLR_RT_TypedArray_UINT8 param1, HRESULT &hr )
{

    (void)param0;
    (void)param1;
    (void)hr;


    ////////////////////////////////
    // implementation starts here //

    // param0 is register address
    // param1 is response array

    param1[0] = 0x01;
    param1[1] = 0x02;

    // implementation ends here   //
    ////////////////////////////////
}
