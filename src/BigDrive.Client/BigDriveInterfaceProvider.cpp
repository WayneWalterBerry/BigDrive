// <copyright file="BigDriveInterfaceProvider.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comdef.h>
#include <iostream>

// Header
#include "BigDriveInterfaceProvider.h"

// Shared
#include "..\Shared\EventLogger.h"

// Initialize the static EventLogger instance
EventLogger BigDriveInterfaceProvider::s_eventLogger(L"BigDrive.Client");

/// <summary>
/// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified CLSID.
/// </summary>
/// <param name="clsid">The CLSID of the COM+ class.</param>
BigDriveInterfaceProvider::BigDriveInterfaceProvider(const CLSID& clsid)
    : m_clsid(clsid)
{
}

HRESULT BigDriveInterfaceProvider::GetInterface(const IID& iid, IUnknown** ppIUnknown)
{
    HRESULT hrReturn = S_OK;
    IUnknown* pIUnknown = nullptr;

    if (ppIUnknown == nullptr)
    {
        return E_POINTER; // Return an appropriate error code
    }

    // Create an instance of the COM class
    hrReturn = ::CoCreateInstance(m_clsid, nullptr, CLSCTX_LOCAL_SERVER, iid, reinterpret_cast<void**>(&pIUnknown));
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to create COM instance. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Query for the requested interface
    hrReturn = pIUnknown->QueryInterface(iid, reinterpret_cast<void**>(ppIUnknown));
    if (FAILED(hrReturn))
    {
        hrReturn = S_FALSE;
        goto End;
    }

End:

    // Release the IUnknown pointer
    if (pIUnknown)
    {
        pIUnknown->Release();
        pIUnknown = nullptr;
    }

    return hrReturn;
}

HRESULT BigDriveInterfaceProvider::GetIBigDriveConfiguration(IBigDriveConfiguration** ppBigDriveConfiguration)
{
    HRESULT hrReturn = S_OK;

    if (ppBigDriveConfiguration == nullptr)
    {
        return E_POINTER; // Return an appropriate error code
    }

    // Get the IBigDriveConfiguration interface
    hrReturn = GetInterface(IID_IBigDriveConfiguration, reinterpret_cast<IUnknown**>(ppBigDriveConfiguration));
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to get IBigDriveConfiguration interface. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

End:
    return hrReturn;
}