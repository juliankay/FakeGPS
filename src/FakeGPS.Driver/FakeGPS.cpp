// 2016 OK

/*++

Module:

    FakeGPS.cpp

Description:

    This module contains the implementation for the sensor service driver device callback object.

--*/

#include "Internal.h"
#include "FakeGPS.h"
#include "FakeGPS.tmh"
#include "Queue.h"

#include <setupapi.h>
#include <devpkey.h>
#include <strsafe.h>

/*++

CFakeGPS::CFakeGPS

    Object constructor function

--*/
CFakeGPS::CFakeGPS() :
    m_spWdfDevice(NULL),
    m_pSensorManager(NULL),
    m_spQueue(nullptr),
    m_dwShutdownControlFlags(0)
{
}

/*++

CFakeGPS::~CFakeGPS

    Object destructor function

--*/
CFakeGPS::~CFakeGPS()
{
    SAFE_RELEASE(m_pSensorManager);
}

/*++

CFakeGPS::CreateInstance

    This static method is used to create and initialize an instance of
    CFakeGPS for use with a given hardware device.

Parameters:

    pDeviceInit - pointer to an interface used to initialize the device
    pDriver     - pointer to an IWDFDriver interface

Return Values:
    S_OK: object created successfully

--*/
HRESULT CFakeGPS::CreateInstance(
    _In_  IWDFDriver*           pDriver,
    _In_  IWDFDeviceInitialize* pDeviceInit)
{
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    CComObject<CFakeGPS>* pFakeGPS = NULL;

    HRESULT hr = CComObject<CFakeGPS>::CreateInstance(&pFakeGPS);

    if (SUCCEEDED(hr) && (nullptr != pFakeGPS))
    {
        pFakeGPS->AddRef();

        // Prepare device parameters
        pDeviceInit->SetLockingConstraint(None);
        pDeviceInit->SetPowerPolicyOwnership(TRUE);             // Power policy
        //pDeviceInit->SetFilter();                             // If you're writing a filter driver then set this flag
        //pDeviceInit->AutoForwardCreateCleanupClose(WdfTrue);

        CComPtr<IUnknown> spCallback;
        hr = pFakeGPS->QueryInterface(IID_IUnknown, (void**) &spCallback);

        CComPtr<IWDFDevice> spIWDFDevice;
        if (SUCCEEDED(hr))
        {
            // Create the IWDFDevice object
            hr = pDriver->CreateDevice(pDeviceInit, spCallback, &spIWDFDevice);
        }

        if (SUCCEEDED(hr))
        {
            // Apply power policy settings
            WUDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;

            WUDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(
                &idleSettings,
                SENSOR_POWER_POLICY_S0_IDLE_CAPABILITIES);

            idleSettings.IdleTimeout = SENSOR_POWER_POLICY_IDLE_TIMEOUT;
            idleSettings.ExcludeD3Cold = SENSOR_POWER_POLICY_EXCLUDE_D3_COLD;

            CComPtr<IWDFDevice3> spIWDFDevice3;
            hr = spIWDFDevice->QueryInterface(IID_PPV_ARGS(&spIWDFDevice3));

            if (SUCCEEDED(hr))
            {
                hr = spIWDFDevice3->AssignS0IdleSettingsEx(&idleSettings);
            }
        }

        // Release the pFakeGPS pointer when done. Note: UMDF holds a reference to it above
        SAFE_RELEASE(pFakeGPS);
    }

    return hr;
}

/*++

CFakeGPS::OnPrepareHardware

    Called by UMDF to prepare the hardware for use. In our case
    we create the SensorDDI object and initialize the Sensor Class Extension

Parameters:
    pWdfDevice - pointer to an IWDFDevice object representing the device

Return Values:
    S_OK: success

--*/
HRESULT CFakeGPS::OnPrepareHardware(
    _In_ IWDFDevice* pWdfDevice)
{
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = (NULL != pWdfDevice) ? S_OK : E_UNEXPECTED;

    if (SUCCEEDED(hr))
    {
        hr = EnterProcessing(PROCESSING_IPNPCALLBACKHARDWARE);

        if (SUCCEEDED(hr))
        {
            if (NULL != pWdfDevice)
            {
                // Store the IWDFDevice pointer
                m_spWdfDevice = pWdfDevice;
            }

            // Create & Configure the default IO Queue
            if (SUCCEEDED(hr))
            {
                hr = ConfigureQueue();
            }

            // Create the sensor manager object
            if (SUCCEEDED(hr))
            {
                hr = CComObject<CSensorManager>::CreateInstance(&m_pSensorManager);

                if (nullptr != m_pSensorManager)
                {
                    if ((SUCCEEDED(hr)) && (NULL != m_pSensorManager))
                    {
                        m_pSensorManager->AddRef();
                    }

                    // Initialize the sensor manager object
                    if (SUCCEEDED(hr))
                    {
                        hr = m_pSensorManager->Initialize(m_spWdfDevice, this);
                    }
                }
                else
                {
                    hr = E_POINTER;
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = StringCchCopy(m_pSensorManager->m_wszDeviceName, MAX_PATH, DEFAULT_DEVICE_MODEL_VALUE);

                if (SUCCEEDED(hr))
                {
                    ULONG ulCchInstanceId = 0;
                    BOOL  fResult = FALSE;
                    WCHAR* wszInstanceId = nullptr;
                    WCHAR* tempStr = nullptr;

                    try
                    {
                        wszInstanceId = new WCHAR[MAX_PATH];
                    }
                    catch (...)
                    {
                        hr = E_UNEXPECTED;
                        Trace(TRACE_LEVEL_ERROR, "Failed to allocate memory for instance ID string, hr = %!HRESULT!", hr);

                        if (nullptr != wszInstanceId)
                        {
                            delete [] wszInstanceId;
                        }
                    }

                    try
                    {
                        tempStr = new WCHAR[MAX_PATH];
                    }
                    catch (...)
                    {
                        hr = E_UNEXPECTED;
                        Trace(TRACE_LEVEL_ERROR, "Failed to allocate memory for instance ID temp string, hr = %!HRESULT!", hr);

                        if (nullptr != tempStr)
                        {
                            delete [] tempStr;
                        }
                    }

                    if (SUCCEEDED(pWdfDevice->RetrieveDeviceInstanceId(NULL, &ulCchInstanceId)))
                    {
                        if (SUCCEEDED(pWdfDevice->RetrieveDeviceInstanceId(wszInstanceId, &ulCchInstanceId)))
                        {
                            HDEVINFO hDeviceInfo = INVALID_HANDLE_VALUE;

                            if (INVALID_HANDLE_VALUE != (hDeviceInfo = ::SetupDiCreateDeviceInfoList(NULL, NULL)))
                            {
                                SP_DEVINFO_DATA deviceInfo = { sizeof(SP_DEVINFO_DATA) };

                                if (TRUE == ::SetupDiOpenDeviceInfo(hDeviceInfo, wszInstanceId, NULL, 0, &deviceInfo))
                                {
                                    DEVPROPTYPE propType;
                                    ULONG ulSize;

                                    fResult = ::SetupDiGetDeviceProperty(hDeviceInfo, &deviceInfo, &DEVPKEY_Device_DeviceDesc, &propType, (PBYTE) tempStr, MAX_PATH*sizeof(WCHAR), &ulSize, 0);

                                    if (FALSE == fResult)
                                    {
                                        hr = HRESULT_FROM_WIN32(GetLastError());
                                    }

#pragma warning(suppress: 26035) //possible failure to null terminate string

                                    if (SUCCEEDED(hr) && (wcscmp(tempStr, L"") != 0))
                                    {
                                        wcscpy_s(m_pSensorManager->m_wszDeviceName, MAX_PATH, tempStr);
                                    }

                                    ::SetupDiDestroyDeviceInfoList(hDeviceInfo);
                                }
                            }
                        }
                    }

#pragma warning(suppress: 6001) //using unitialized memory

                    if (nullptr != wszInstanceId)
                    {
                        delete [] wszInstanceId;
                    }

#pragma warning(suppress: 6001) //using uninitialized memory

                    if (nullptr != tempStr)
                    {
                        delete [] tempStr;
                    }
                }
            }

        } // processing in progress

        ExitProcessing(PROCESSING_IPNPCALLBACKHARDWARE);
    }

    if (FAILED(hr))
    {
        Trace(TRACE_LEVEL_CRITICAL, "Abnormal results during hardware initialization, hr = %!HRESULT!", hr);
    }

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);

    return hr;
}

/*++

CFakeGPS::OnReleaseHardware

    Called by UMDF to uninitialize the hardware.

Parameters:
    pWdfDevice - pointer to an IWDFDevice object for the device

Return Values:
    S_OK:

--*/
HRESULT CFakeGPS::OnReleaseHardware(
    _In_ IWDFDevice* pWdfDevice)
{
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = S_OK;

    EnterShutdown();

    // Stop the device and uninitialize the sensor manager object
    if (NULL != m_pSensorManager)
    {
        hr = m_pSensorManager->Stop();

        m_pSensorManager->Uninitialize();
        SAFE_RELEASE(m_pSensorManager);
    }

    // Release the IWDFDevice handle, if it matches
    if (pWdfDevice == m_spWdfDevice.p)
    {
        m_spWdfDevice.Release();
    }

    ExitShutdown();


    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);

    Trace(TRACE_LEVEL_CRITICAL, "FakeGPS - Trace log ending for this instance of driver");
    Trace(TRACE_LEVEL_CRITICAL, "------------------------------- END -------------------------------------------");

    return hr;
}

/*++

CFakeGPS::OnD0Entry

    This method is called after a new device enters the system

Parameters:
    pWdfDevice      - pointer to a device object
    prwviousSate    - power device state

--*/
HRESULT CFakeGPS::OnD0Entry(
    _In_ IWDFDevice* pWdfDevice,
    _In_ WDF_POWER_DEVICE_STATE previousState)
{
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    UNREFERENCED_PARAMETER(pWdfDevice);
    UNREFERENCED_PARAMETER(previousState);

    HRESULT hr = S_OK;

    hr = EnterProcessing(PROCESSING_IPNPCALLBACK);

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr) && NULL != m_pSensorManager)
        {
            hr = m_pSensorManager->Start();

            if (FAILED(hr))
            {
                Trace(TRACE_LEVEL_CRITICAL, "Failed to initialize sensors, hr = %!HRESULT!", hr);
            }
        }

    } // processing in progress

    ExitProcessing(PROCESSING_IPNPCALLBACK);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);

    return hr;
}

/*++

CFakeGPS::OnD0Exit

    This method is called when a device leaves the system

Parameters:
    pWdfDevice  - pointer to a device object
    newState    - power device state

--*/
HRESULT CFakeGPS::OnD0Exit(
    _In_ IWDFDevice* pWdfDevice,
    _In_ WDF_POWER_DEVICE_STATE newState)
{
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    UNREFERENCED_PARAMETER(pWdfDevice);
    UNREFERENCED_PARAMETER(newState);

    HRESULT hr = S_OK;

    hr = EnterProcessing(PROCESSING_IPNPCALLBACK);

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr) && NULL != m_pSensorManager)
        {
            hr = m_pSensorManager->Stop();
        }

    } // processing in progress

    ExitProcessing(PROCESSING_IPNPCALLBACK);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);

    return hr;
}

VOID CFakeGPS::OnSurpriseRemoval(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    EnterShutdown();

    return;
}

HRESULT CFakeGPS::OnQueryRemove(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    return S_OK;
}

HRESULT CFakeGPS::OnQueryStop(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    return S_OK;
}

/*++

CFakeGPS::OnCleanupFile

    This method is called when the file handle to the device is closed

Parameters:
    pWdfFile - pointer to a file object

--*/
VOID CFakeGPS::OnCleanupFile(
    _In_ IWDFFile* pWdfFile)
{
    //Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = S_OK;

    hr = EnterProcessing(PROCESSING_IFILECALLBACKCLEANUP);

    if (SUCCEEDED(hr))
    {
        if (NULL != m_pSensorManager)
        {
            m_pSensorManager->CleanupFile(pWdfFile);
        }

    } // processing in progress

    ExitProcessing(PROCESSING_IFILECALLBACKCLEANUP);

    return;
}

/*++

CFakeGPS::ConfigureQueue

    This method is called after the device callback object has been initialized
    and returned to the driver.  It would setup the device's queues and their
    corresponding callback objects.

Parameters:

Return Values:
    S_OK: success

--*/
HRESULT CFakeGPS::ConfigureQueue()
{
    HRESULT hr = S_OK;

    CComPtr<IWDFIoQueue> spIoQueue;

    if (NULL != m_spWdfDevice)
    {
        m_spWdfDevice->GetDefaultIoQueue(&spIoQueue);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr) && (NULL == spIoQueue))
    {
        hr = CMyQueue::CreateInstance(m_spWdfDevice, this, &m_spQueue);
    }

    return hr;
}

/*++

CFakeGPS::ProcessIoControl

    This method is a helper that takes the incoming IOCTL and forwards
    it to the Windows Sensor Class Extension for processing.

Parameters:
    pQueue                  - [in] pointer to the UMDF queue that handled the request
    pRequest                - [in] pointer to the request
    ControlCode             - [in] the IOCTL code
    InputBufferSizeInBytes  - [in] size of the incoming IOCTL buffer
    OutputBufferSizeInBytes - [out] size of the outgoing IOCTL buffer
    pcbWritten              - pointer to a DWORD containing the number of bytes returned

Return Values:
    S_OK:

--*/
HRESULT CFakeGPS::ProcessIoControl(
    _In_ IWDFIoQueue*   pQueue,
    _In_ IWDFIoRequest* pRequest,
    _In_ ULONG          ControlCode,
    SIZE_T              InputBufferSizeInBytes,
    SIZE_T              OutputBufferSizeInBytes,
    DWORD*              pcbWritten)
{
    UNREFERENCED_PARAMETER(pQueue);
    UNREFERENCED_PARAMETER(ControlCode);
    UNREFERENCED_PARAMETER(InputBufferSizeInBytes);
    UNREFERENCED_PARAMETER(OutputBufferSizeInBytes);
    UNREFERENCED_PARAMETER(pcbWritten);

    HRESULT hr = S_OK;

    if (NULL != m_pSensorManager)
    {
        hr = m_pSensorManager->ProcessIoControl(pRequest);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

/*++

CFakeGPS::ProcessIoControlRadioManagement

    This method processes IO for Radio Management.  The radio state is
    either read or set on the sensor.

Parameters:
    pRequest                - [in] pointer to the request
    ControlCode             - [in] the IOCTL code

Return Values:
    S_OK:

--*/
HRESULT CFakeGPS::ProcessIoControlRadioManagement(
    _In_ IWDFIoRequest*   pRequest,
    _In_ ULONG            ControlCode)
{
    HRESULT hr = S_OK;

    if (nullptr != m_pSensorManager)
    {
        hr = m_pSensorManager->ProcessIoControlRadioManagement(pRequest, ControlCode);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}


inline HRESULT CFakeGPS::EnterProcessing(DWORD64 dwControlFlag)
{
    //Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = S_OK;

    if ((InterlockedOr(&m_dwShutdownControlFlags, dwControlFlag) & SHUTDOWN_IN_PROGRESS) != 0)
    {
        hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    return hr;
}

inline void CFakeGPS::ExitProcessing(DWORD64 dwControlFlag)
{
    //Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    InterlockedAnd(&m_dwShutdownControlFlags, ~dwControlFlag);
}

inline void CFakeGPS::EnterShutdown()
{
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    // Begin shutdown.  Spin if control handler is in progress.
    while (((InterlockedOr(&m_dwShutdownControlFlags, SHUTDOWN_IN_PROGRESS) & PROCESSING_IN_PROGRESS) != 0))
    {
        Yield();
    }
}

inline void CFakeGPS::ExitShutdown()
{
    // End shutdown. Clear the flag.
    InterlockedAnd(&m_dwShutdownControlFlags, ~SHUTDOWN_IN_PROGRESS);
}

/*++

CFakeGPS::OnSelfManagedIoCleanup

    Called by UMDF to release memory for a device's self-managed I/O
    operations, after the device is removed.

Parameters:
    pWdfDevice  - pointer to an IWDFDevice object representing the device

Return Values:

--*/
VOID CFakeGPS::OnSelfManagedIoCleanup(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
}

/*++

CFakeGPS::OnSelfManagedIoFlush

    Called by UMDF to flush the device for a device's self-managed I/O operations.

Parameters:
    pWdfDevice  - pointer to an IWDFDevice object representing the device

Return Values:

--*/
VOID CFakeGPS::OnSelfManagedIoFlush(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = EnterProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    if (SUCCEEDED(hr))
    {
        if (nullptr != m_spQueue)
        {
            Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Purging queue");
            m_spQueue->PurgeSynchronously();
        }
    }

    ExitProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);
}

/*++

CFakeGPS::OnSelfManagedIoInit

    Called by UMDF to initialize a device's self-managed I/O operations.

Parameters:
    pWdfDevice - pointer to an IWDFDevice object representing the device

Return Values:
    S_OK: success

--*/
HRESULT CFakeGPS::OnSelfManagedIoInit(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = EnterProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    if (SUCCEEDED(hr))
    {
        if (nullptr != m_pSensorManager)
        {
            Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Hardware is now available");
            m_pSensorManager->m_fDeviceActive = true;
        }
    }

    ExitProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);
    return hr;
}

/*++

CFakeGPS::OnSelfManagedIoRestart

    Called by UMDF to restart a device's self-managed I/O operations.

Parameters:
    pWdfDevice  - pointer to an IWDFDevice object representing the device

Return Values:
    S_OK: success

--*/
HRESULT CFakeGPS::OnSelfManagedIoRestart(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = EnterProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    if (SUCCEEDED(hr))
    {
        if (nullptr != m_pSensorManager)
        {
            Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Hardware is now available");
            m_pSensorManager->m_fDeviceActive = true;
        }
    }

    ExitProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);
    return hr;
}

/*++

CFakeGPS::OnSelfManagedIoStop

    The OnSelfManagedIoStop method is not used in the current version of the UMDF.

Parameters:
    pWdfDevice - pointer to an IWDFDevice object representing the device

Return Values:
    S_OK: success

--*/
HRESULT CFakeGPS::OnSelfManagedIoStop(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);
    HRESULT hr = S_OK;

    return hr;
}

/*++

CFakeGPS::OnSelfManagedIoSuspend

    Called by UMDF to suspend a device's self-managed I/O operations.

    All outstanding I/O must be completed.  The queue is stopped
    to flush out in progress requests.  The queue is restarted and
    the driver continues to get I/O, but the m_fDeviceActive flag is
    set to false so that the hardware will not be accessed.

Parameters:
    pWdfDevice - pointer to an IWDFDevice object representing the device

Return Values:
    S_OK: success

--*/
HRESULT CFakeGPS::OnSelfManagedIoSuspend(
    _In_ IWDFDevice* pWdfDevice)
{
    UNREFERENCED_PARAMETER(pWdfDevice);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Entry");

    HRESULT hr = EnterProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    if (SUCCEEDED(hr))
    {
        if (nullptr != m_pSensorManager)
        {
            Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Hardware is now not available");

            m_pSensorManager->m_fDeviceActive = false;
        }

        if (nullptr != m_spQueue)
        {
            Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Flushing queue of running requests");

            // TODO: If all hardware requests are not guaranteed to complete within 1 second then
            // the queue should be stopped and all pending hardware requests canceled
            //m_spQueue->Stop(nullptr); // Uncomment this line if the hardware requests need to be canceled

            // As noted above, cancel all pending hardware requests here to allow all ISensorDriver:: callbacks to complete

            m_spQueue->StopSynchronously();

            // NOTE Any asynchronous work that accesses the hardware should be stopped

            m_spQueue->Start();
        }
    }

    ExitProcessing(PROCESSING_IPNPCALLBACKSELFMANAGEDIO);

    Trace(TRACE_LEVEL_INFORMATION, "%!FUNC! Exit, hr = %!HRESULT!", hr);

    return hr;
}