// <copyright file="BigDriveShellFolderFactory-IClassFactory.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderFactory.h"
#include "BigDriveShellFolderTraceLogger.h"

#include <windows.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// IClassFactory methods

/// <inheritdoc />
HRESULT __stdcall BigDriveShellFolderFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
    HRESULT hr = S_OK;
    BigDriveShellFolder* pFolder = nullptr;

    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__, riid);

    if (!ppvObject) 
    {
        hr = E_POINTER;
        goto End;
    }

    if (pUnkOuter != nullptr)
    {
        // Aggregation is not supported
        hr = CLASS_E_NOAGGREGATION;
        goto End;
    }

    *ppvObject = nullptr;

    // Create an instance of BigDriveFolder
    hr = BigDriveShellFolder::Create(m_driveGuid, nullptr, nullptr, &pFolder);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"CreateInstance: Create() Failed. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Query the requested interface
    hr = pFolder->QueryInterface(riid, ppvObject);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"CreateInstance: QueryInterface() Failed. HRESULT: 0x%08X", hr);
        goto End;
    }

End:

    if (pFolder != nullptr)
    {
        pFolder->Release();
        pFolder = nullptr;
    }

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveShellFolderFactory::LockServer(BOOL fLock) 
{
    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__);

    // Lock or unlock the server
    if (fLock) 
    {
        ::InterlockedIncrement(&m_refCount);
    }
    else 
    {
        ::InterlockedDecrement(&m_refCount);
    }

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, S_OK);

    return S_OK;
}