// <copyright file="BigDriveShellFolderFactory.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderFactory.h"
#include "BigDriveETWLogger.h"

// Define the static member outside the class
PIDLIST_ABSOLUTE BigDriveShellFolderFactory::s_pidlRoot = ILCreateFromPathW(L"::");

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// IClassFactory methods

/// <inheritdoc />
HRESULT __stdcall BigDriveShellFolderFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
    HRESULT hr = S_OK;

    BigDriveShellFolder* pFolder = nullptr;

    BigDriveETWLogger::LogEnter(__FUNCTION__, __LINE__);

    if (pUnkOuter != nullptr)
    {
        // Aggregation is not supported
        hr = CLASS_E_NOAGGREGATION;
        goto End;
    }

    // Create an instance of BigDriveFolder
    pFolder = new (std::nothrow) BigDriveShellFolder(m_driveGuid, nullptr, s_pidlRoot);
    if (!pFolder)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Query the requested interface
    hr = pFolder->QueryInterface(riid, ppvObject);
    if (FAILED(hr))
    {
        goto End;
    }

End:

    BigDriveETWLogger::LogLeave(__FUNCTION__, __LINE__, hr);

    if (pFolder != nullptr)
    {
        pFolder->Release();
        pFolder = nullptr;
    }

    return hr;
}