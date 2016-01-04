// 2016 OK

/*++

Module:

    Queue.h

Description:

    This module contains the type definitions for the sensors service driver queue callback class.

--*/

#pragma once

class ATL_NO_VTABLE CMyQueue :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IQueueCallbackDeviceIoControl
{

public:

    virtual ~CMyQueue();

    DECLARE_NOT_AGGREGATABLE(CMyQueue)

    BEGIN_COM_MAP(CMyQueue)
        COM_INTERFACE_ENTRY(IQueueCallbackDeviceIoControl)
    END_COM_MAP()

    static HRESULT CreateInstance(
        _In_ IWDFDevice* pWdfDevice,
        CFakeGPS* pFakeGPS,
        IWDFIoQueue** ppQueue);

protected:

    CMyQueue();

    // COM Interface methods
public:

    // IQueueCallbackDeviceIoControl
    STDMETHOD_(void, OnDeviceIoControl)(
        _In_ IWDFIoQueue*    pQueue,
        _In_ IWDFIoRequest*  pRequest,
        _In_ ULONG           ControlCode,
        SIZE_T          InputBufferSizeInBytes,
        SIZE_T          OutputBufferSizeInBytes);

private:
    inline HRESULT EnterProcessing(DWORD64 dwControlFlag);
    inline void    ExitProcessing(DWORD64 dwControlFlag);

    CFakeGPS*          m_pParentDevice; // Parent device object
};