// 2016 OK

/*++

Module:

    Driver.cpp

Description:

    This module contains the implementation for the sensors service driver callback class.

--*/

#include "Internal.h"
#include "FakeGPSLib.h" // IDL Generated File
#include "Driver.tmh"
#include "Driver.h"
#include "FakeGPS.h"

/*++

CMyDriver::CMyDriver

    Object constructor function

--*/
CMyDriver::CMyDriver()
{
}

/*++

CMyDriver::OnDeviceAdd

    The framework call this function when device is detected. This driver creates a device callback object

Parameters:

    pDriver     - pointer to an IWDFDriver object
    pDeviceInit - pointer to a device initialization object

Return Values:

    S_OK: device initialized successfully

--*/
HRESULT CMyDriver::OnDeviceAdd(
    _In_ IWDFDriver* pDriver,
    _In_ IWDFDeviceInitialize* pDeviceInit)
{
    // informational at the start of the run in case there are no other log entries to view
    Trace(TRACE_LEVEL_CRITICAL, "   ");
    Trace(TRACE_LEVEL_CRITICAL, "------------------------------ START ------------------------------------------");
    Trace(TRACE_LEVEL_CRITICAL, "FakeGPS - Trace log running with TRACE_LEVEL == CRITICAL");
    Trace(TRACE_LEVEL_ERROR, "FakeGPS - Trace log running with TRACE_LEVEL == ERROR");
    Trace(TRACE_LEVEL_WARNING, "FakeGPS - Trace log running with TRACE_LEVEL == WARNING");
    Trace(TRACE_LEVEL_INFORMATION, "FakeGPS - Trace log running with TRACE_LEVEL == INFORMATION");
    Trace(TRACE_LEVEL_VERBOSE, "FakeGPS - Trace log running with TRACE_LEVEL == VERBOSE");

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = CFakeGPS::CreateInstance(pDriver, pDeviceInit);

    return hr;
}

/*++

CMyDriver::OnInitialize

    The framework calls this function just after loading the driver. The driver
    can perform any global, device independent initialization in this routine.

--*/
HRESULT CMyDriver::OnInitialize(
    _In_ IWDFDriver* pDriver)
{
    UNREFERENCED_PARAMETER(pDriver);
    return S_OK;
}

/*++

CMyDriver::OnDeinitialize

    The framework calls this function just before de-initializing itself. All
    WDF framework resources should be released by driver before returning
    from this call.

--*/
void CMyDriver::OnDeinitialize(
    _In_ IWDFDriver* pDriver)
{
    UNREFERENCED_PARAMETER(pDriver);
    return;
}