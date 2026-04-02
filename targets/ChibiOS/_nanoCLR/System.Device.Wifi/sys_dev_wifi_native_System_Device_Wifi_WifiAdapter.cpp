//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include <sys_dev_wifi_native.h>
#include <nf_rt_events_native.h>

///////////////////////////////////////////////////////////////////////////////////////
// !!! KEEP IN SYNC WITH System.Device.WiFi (in managed code) !!! //
///////////////////////////////////////////////////////////////////////////////////////
struct ScanRecord
{
    uint8_t bssid[6];
    uint8_t ssid[33];
    uint8_t rssi;
    uint8_t authMode;
    uint8_t cypherType;
};

HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::NativeSetDeviceName___VOID__STRING(
    CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();
    {
        (void)stack;
        // Not supported on this platform — hostname is set via lwIP directly.
        NANOCLR_SET_AND_LEAVE(CLR_E_NOT_SUPPORTED);
    }
    NANOCLR_NOCLEANUP();
}

HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::DisposeNative___VOID(CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();
    (void)stack;

    NANOCLR_NOCLEANUP_NOLABEL();
}

HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::NativeInit___VOID(CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();
    (void)stack;

    // WICED is initialised in main.c (wwd_buffer_init + wwd_management_wifi_on).
    // Nothing extra to do here.

    NANOCLR_NOCLEANUP_NOLABEL();
}

//
//  Pickup Net interface index and do checks
//
HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::GetNetInterfaceIndex(
    CLR_RT_StackFrame &stack,
    int *pNetIndex)
{
    NANOCLR_HEADER();
    {
        CLR_RT_HeapBlock *pThis = stack.This();
        FAULT_ON_NULL(pThis);

        if (pThis[Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::FIELD___disposedValue]
                .NumericByRef()
                .u1 != 0)
        {
            NANOCLR_SET_AND_LEAVE(CLR_E_OBJECT_DISPOSED);
        }

        *pNetIndex = pThis[FIELD___networkInterface].NumericByRefConst().s4;
    }
    NANOCLR_NOCLEANUP();
}

//
// Connect to Wireless connection using passed SSID / passPhrase
//
HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::
    NativeConnect___SystemDeviceWifiWifiConnectionStatus__STRING__STRING__SystemDeviceWifiWifiReconnectionKind(
        CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();
    {
        const char *szSsid;
        const char *szPassPhrase;
        int reconnectionKind;
        int netIndex;
        CLR_RT_HeapBlock hbTimeout;
        CLR_INT64 *timeout;
        bool eventResult = true;
        WifiConnectionStatus Status = WifiConnectionStatus_UnspecifiedFailure;

        NANOCLR_CHECK_HRESULT(GetNetInterfaceIndex(stack, &netIndex));

        if (stack.m_customState == 0)
        {
            NANOCLR_CHECK_HRESULT(GetNetInterfaceIndex(stack, &netIndex));

            // Get SSID
            szSsid = stack.Arg1().RecoverString();
            FAULT_ON_NULL(szSsid);

            // Get Password
            szPassPhrase = stack.Arg2().RecoverString();
            FAULT_ON_NULL(szPassPhrase);

            // Reconnect kind
            reconnectionKind = stack.Arg3().NumericByRef().s4;

            // Set timeout for connect (20 seconds)
            hbTimeout.SetInteger((CLR_INT64)20000 * TIME_CONVERSION__TO_MILLISECONDS);

            int res = Network_Interface_Start_Connect(netIndex, szSsid, szPassPhrase, reconnectionKind);
            if (res != 0)
            {
                Status = WifiConnectionStatus_UnspecifiedFailure;
                eventResult = false;
            }
        }

        // Wait for connect to finish
        while (eventResult)
        {
            int connectResult = Network_Interface_Connect_Result(netIndex);
            if (connectResult >= 0)
            {
                switch (connectResult)
                {
                    case 0:
                        Status = WifiConnectionStatus_Success;
                        break;

                    case 1:
                        Status = WifiConnectionStatus_NetworkNotAvailable;
                        break;

                    case 2:
                        Status = WifiConnectionStatus_InvalidCredential;
                        break;

                    default:
                        Status = WifiConnectionStatus_UnspecifiedFailure;
                        break;
                }
                break;
            }

            // Get timeout
            NANOCLR_CHECK_HRESULT(stack.SetupTimeoutFromTicks(hbTimeout, timeout));

            // non-blocking wait allowing other threads to run while we wait for the connect to complete
            NANOCLR_CHECK_HRESULT(
                g_CLR_RT_ExecutionEngine.WaitEvents(stack.m_owningThread, *timeout, Event_Wifi_Station, eventResult));

            if (!eventResult)
            {
                // Timeout
                Status = WifiConnectionStatus_Timeout;
                break;
            }
        }

        // Return value to the managed application
        stack.SetResult_I4(Status);
    }
    NANOCLR_NOCLEANUP();
}

//
//  Disconnect WiFi connection
//
HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::NativeDisconnect___VOID(CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();
    {
        int netIndex;

        NANOCLR_CHECK_HRESULT(GetNetInterfaceIndex(stack, &netIndex));

        Network_Interface_Disconnect(netIndex);
    }
    NANOCLR_NOCLEANUP();
}

HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::NativeScanAsync___VOID(CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();
    {
        int netIndex;

        NANOCLR_CHECK_HRESULT(GetNetInterfaceIndex(stack, &netIndex));

        // Start scan — scan is not yet implemented for WICED/ChibiOS
        int startScanResult = Network_Interface_Start_Scan(netIndex);
        if (startScanResult != 0)
        {
            NANOCLR_SET_AND_LEAVE(CLR_E_NOT_SUPPORTED);
        }
    }
    NANOCLR_NOCLEANUP();
}

// Returns scan results as a byte array
HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::GetNativeScanReport___SZARRAY_U1(
    CLR_RT_StackFrame &stack)
{
    (void)stack;
    NANOCLR_HEADER();

    // Scan not yet implemented for WICED/ChibiOS — return empty report
    {
        CLR_RT_HeapBlock &top = stack.PushValueAndClear();

        // Minimum valid report: 2 bytes (record count = 0)
        NANOCLR_CHECK_HRESULT(CLR_RT_HeapBlock_Array::CreateInstance(top, 2, g_CLR_RT_WellKnownTypes.m_UInt8));
        CLR_RT_HeapBlock_Array *array = top.DereferenceArray();
        CLR_UINT8 *buf = array->GetFirstElement();
        buf[0] = 0; // record count
        buf[1] = 0;
    }

    NANOCLR_NOCLEANUP();
}

HRESULT Library_sys_dev_wifi_native_System_Device_Wifi_WifiAdapter::NativeFindWirelessAdapters___STATIC__SZARRAY_U1(
    CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();

    CLR_RT_HeapBlock_Array *array;
    CLR_UINT8 *arrayOfIndexes;
    int index;
    int interfaceCount = 0;

    // add return object to stack
    CLR_RT_HeapBlock &top = stack.PushValueAndClear();

    HAL_Configuration_NetworkInterface *netInterfaceConfig =
        (HAL_Configuration_NetworkInterface *)platform_malloc(sizeof(HAL_Configuration_NetworkInterface));

    // check allocation
    if (netInterfaceConfig == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_OUT_OF_MEMORY);
    }

    // 1st pass: find number of wireless adapters
    for (index = 0; index < g_TargetConfiguration.NetworkInterfaceConfigs->Count; index++)
    {
        if (!ConfigurationManager_GetConfigurationBlock(netInterfaceConfig, DeviceConfigurationOption_Network, index))
        {
            NANOCLR_SET_AND_LEAVE(CLR_E_FAIL);
        }

        // check if this is a Wireless80211 adapter
        if (netInterfaceConfig->InterfaceType == NetworkInterfaceType_Wireless80211)
        {
            interfaceCount++;
        }
    }

    // build array with indexes of Wireless interfaces
    NANOCLR_CHECK_HRESULT(CLR_RT_HeapBlock_Array::CreateInstance(top, interfaceCount, g_CLR_RT_WellKnownTypes.m_UInt8));
    array = top.DereferenceArray();
    arrayOfIndexes = array->GetFirstElement();

    // 2nd pass: grab the index of the wireless adapters
    for (index = 0; index < g_TargetConfiguration.NetworkInterfaceConfigs->Count; index++)
    {
        if (!ConfigurationManager_GetConfigurationBlock(netInterfaceConfig, DeviceConfigurationOption_Network, index))
        {
            NANOCLR_SET_AND_LEAVE(CLR_E_FAIL);
        }

        // check if this is a Wireless80211 adapter
        if (netInterfaceConfig->InterfaceType == NetworkInterfaceType_Wireless80211)
        {
            // store index
            *arrayOfIndexes = index;

            // move to next position in array of indexes
            arrayOfIndexes++;
        }
    }

    NANOCLR_NOCLEANUP();
}
