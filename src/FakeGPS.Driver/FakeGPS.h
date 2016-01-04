// 2016 OK

/*++

Module:

    FakeGPS.h

Description:

    This module contains the type definitions for the sensor service driver device callback class.

--*/

#pragma once

#include "SensorManager.h"

class ATL_NO_VTABLE CFakeGPS :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IPnpCallback,
    public IPnpCallbackHardware,
    public IPnpCallbackSelfManagedIo,
    public IFileCallbackCleanup
{
public:
    ~CFakeGPS();

    DECLARE_NOT_AGGREGATABLE(CFakeGPS)

    BEGIN_COM_MAP(CFakeGPS)
        COM_INTERFACE_ENTRY(IPnpCallback)
        COM_INTERFACE_ENTRY(IPnpCallbackHardware)
        COM_INTERFACE_ENTRY(IPnpCallbackSelfManagedIo)
        COM_INTERFACE_ENTRY(IFileCallbackCleanup)
    END_COM_MAP()

protected:

    CFakeGPS();

    HRESULT ConfigureQueue();

    // COM Interface methods
public:

    // IPnpCallbackHardware
    STDMETHOD_(HRESULT, OnPrepareHardware)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnReleaseHardware)(_In_ IWDFDevice* pWdfDevice);

    // IPnpCallback
    STDMETHOD(OnD0Entry)(_In_ IWDFDevice* pWdfDevice, _In_ WDF_POWER_DEVICE_STATE previousState);
    STDMETHOD(OnD0Exit)(_In_ IWDFDevice* pWdfDevice, _In_ WDF_POWER_DEVICE_STATE newState);
    STDMETHOD_(VOID, OnSurpriseRemoval)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnQueryRemove)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnQueryStop)(_In_ IWDFDevice* pWdfDevice);

    // IFileCallbackCleanup
    STDMETHOD_(VOID, OnCleanupFile)(_In_ IWDFFile *pWdfFile);

    // IPnpCallbackSelfManagedIo
    STDMETHOD_(VOID, OnSelfManagedIoCleanup)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(VOID, OnSelfManagedIoFlush)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnSelfManagedIoInit)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnSelfManagedIoRestart)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnSelfManagedIoStop)(_In_ IWDFDevice* pWdfDevice);
    STDMETHOD_(HRESULT, OnSelfManagedIoSuspend)(_In_ IWDFDevice* pWdfDevice);

public:

    // The factory method used to create an instance of this device
    static HRESULT CreateInstance(
        _In_ IWDFDriver*            pDriver,
        _In_ IWDFDeviceInitialize*  pDeviceInit);

    HRESULT ProcessIoControl(
        _In_ IWDFIoQueue*       pQueue,
        _In_ IWDFIoRequest*     pRequest,
        _In_ ULONG              ControlCode,
        SIZE_T                  InputBufferSizeInBytes,
        SIZE_T                  OutputBufferSizeInBytes,
        DWORD*                  pcbWritten);

    HRESULT ProcessIoControlRadioManagement(
        _In_ IWDFIoRequest*     pRequest,
        _In_ ULONG              ControlCode);

    inline HRESULT EnterProcessing(DWORD64 dwControlFlag);
    inline void    ExitProcessing(DWORD64 dwControlFlag);

private:

    inline void     EnterShutdown();
    inline void     ExitShutdown();

    CComPtr<IWDFDevice>                 m_spWdfDevice;
    CComObject<CSensorManager>*         m_pSensorManager;
    CComPtr<IWDFIoQueue>                m_spQueue;
    volatile DWORD64                    m_dwShutdownControlFlags;
};