// <copyright file="BigDriveConfigurationClient.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <iostream>
#include <windows.h>
#include <comdef.h>
#include <objbase.h>

#include "BigDriveConfigurationClient.h" 
#include "DriveConfiguration.h"
#include "interfaces/IBigDriveConfiguration.h"

/// </inheritdoc>
HRESULT BigDriveConfigurationClient::GetDriveConfiguration(GUID guid, LPWSTR *pszConfiguration)
{
    HRESULT hrReturn = S_OK;
    IBigDriveConfiguration* pBigDriveConfiguration = nullptr;

    BSTR configuration = nullptr;

    // Initialize COM
    hrReturn = CoInitialize(NULL);
    if (FAILED(hrReturn))
    {
        return hrReturn;
    }

    // Create an instance of the BigDriveConfiguration COM object
    hrReturn = CoCreateInstance(CLSID_BigDriveConfiguration, NULL, CLSCTX_LOCAL_SERVER, IID_IBigDriveConfiguration, (void**)&pBigDriveConfiguration);
    if (FAILED(hrReturn))
    {
        goto End;
    }

    hrReturn = pBigDriveConfiguration->GetConfiguration(guid, &configuration);
    if (!SUCCEEDED(hrReturn))
    {
        goto End;
    }

    // Convert BSTR to LPWSTR
    *pszConfiguration = ::SysAllocString(configuration);
    if (*pszConfiguration == NULL)
    {
        hrReturn = E_OUTOFMEMORY;
        goto End;
    }

End:

    if (configuration)
    {
        ::SysFreeString(configuration);
    }

    if (pBigDriveConfiguration)
    {
        pBigDriveConfiguration->Release();
    }

    // Uninitialize COM
    CoUninitialize();

    return hrReturn;
}

/// </inheritdoc>
HRESULT BigDriveConfigurationClient::GetDriveConfiguration(GUID guid, DriveConfiguration& driveConfiguration)
{
    HRESULT hrReturn = S_OK;
    LPWSTR pszConfiguration = nullptr;

    // Get the configuration from the registry
    hrReturn = GetDriveConfiguration(guid, &pszConfiguration);
    if (FAILED(hrReturn))
    {
        return hrReturn;
    }

    hrReturn = driveConfiguration.ParseJson(pszConfiguration);
    if (FAILED(hrReturn))
    {
        return hrReturn;
    }
    
    // Clean up
    ::SysFreeString(pszConfiguration);
    return hrReturn;
}
