// <copyright file="BigDriveConfigurationClient.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <iostream>
#include <windows.h>
#include <comdef.h>
#include <corerror.h>
#include <objbase.h>

// Header
#include "BigDriveConfigurationClient.h" 

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "DriveConfiguration.h"
#include "interfaces/IBigDriveConfiguration.h"

// Initialize the static EventLogger instance
EventLogger BigDriveConfigurationClient::s_eventLogger(L"BigDrive.Client");

/// </inheritdoc>
HRESULT BigDriveConfigurationClient::GetDriveConfiguration(GUID guid, LPWSTR* pszConfiguration)
{
    HRESULT hr = S_OK;
    IBigDriveConfiguration* pBigDriveConfiguration = nullptr;

    BSTR configuration = nullptr;

    // Initialize COM
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to initialize COM. HRESULT: 0x%08X", hr);
        return hr;
    }

    // Create an instance of the BigDriveConfiguration COM object
    hr = CoCreateInstance(CLSID_BigDriveConfiguration, NULL, CLSCTX_LOCAL_SERVER, IID_IBigDriveConfiguration, (void**)&pBigDriveConfiguration);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to create BigDriveConfiguration COM instance. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pBigDriveConfiguration->GetConfiguration(guid, &configuration);
    switch (hr)
    {
    case S_OK:
        break;
    case COR_E_INVALIDOPERATION:
        // Shell Folder is registered, but drive isn't
        s_eventLogger.WriteErrorFormmated(L"BigDriveConfigurationClient::GetDriveConfiguration - Invalid operation. HRESULT: 0x%08X", hr);
        goto End;
    default:
        s_eventLogger.WriteErrorFormmated(L"BigDriveConfigurationClient::GetDriveConfiguration - Unexpected error. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Convert BSTR to LPWSTR
    *pszConfiguration = ::SysAllocString(configuration);
    if (*pszConfiguration == NULL)
    {
        hr = E_OUTOFMEMORY;
        s_eventLogger.WriteErrorFormmated(L"Failed to allocate memory for configuration string. HRESULT: 0x%08X", hr);
        goto End;
    }

End:

    if (configuration)
    {
        ::SysFreeString(configuration);
        configuration = nullptr;
    }

    if (pBigDriveConfiguration)
    {
        pBigDriveConfiguration->Release();
        pBigDriveConfiguration = nullptr;
    }

    // Uninitialize COM
    CoUninitialize();

    return hr;
}

/// </inheritdoc>
HRESULT BigDriveConfigurationClient::GetDriveConfiguration(GUID guid, DriveConfiguration& driveConfiguration)
{
    HRESULT hr = S_OK;
    LPWSTR pszConfiguration = nullptr;

    // Get the configuration from the registry
    hr = GetDriveConfiguration(guid, &pszConfiguration);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to get drive configuration from registry. HRESULT: 0x%08X", hr);
        return hr;
    }

    hr = driveConfiguration.ParseJson(pszConfiguration);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to parse drive configuration JSON. HRESULT: 0x%08X", hr);
        return hr;
    }

    // Clean up
    ::SysFreeString(pszConfiguration);
    return hr;
}

