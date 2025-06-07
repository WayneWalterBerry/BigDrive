// <copyright file="BigDriveInterfaceProvider.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comdef.h>
#include <iostream>

// Header
#include "BigDriveInterfaceProvider.h"

// Local
#include "BigDriveClientEventLogger.h"
#include "Interfaces/IBigDriveConfiguration.h"
#include "Interfaces/IBigDriveEnumerate.h"
#include "Interfaces/IBigDriveFileInfo.h"

// Initialize the static EventLogger instance
BigDriveClientEventLogger BigDriveInterfaceProvider::s_eventLogger(L"BigDrive.Client");

/// <summary>
/// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified CLSID.
/// </summary>
/// <param name="clsid">The CLSID of the COM+ class.</param>
BigDriveInterfaceProvider::BigDriveInterfaceProvider(const CLSID& clsid)
    : m_clsid(clsid)
{
}

/// <summary>
/// Initializes a new instance of the <see cref="BigDriveInterfaceProvider"/> class with the specified DriveConfiguration.
/// </summary>
/// <param name="driveConfiguration">The drive configuration containing the CLSID of the COM+ class.</param>
BigDriveInterfaceProvider::BigDriveInterfaceProvider(DriveConfiguration& driveConfiguration)
    : m_clsid(driveConfiguration.clsid)
{
}

/// <summary>
/// Retrieves the specified interface from the COM+ class instance.
/// </summary>
/// <param name="iid">The IID of the interface to retrieve.</param>
/// <param name="ppIUnknown">A pointer to the interface pointer to be populated.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveInterfaceProvider::GetInterface(const IID& iid, IUnknown** ppIUnknown)
{
    HRESULT hr = S_OK;
    IUnknown* pIUnknown = nullptr;

    if (ppIUnknown == nullptr)
    {
        return E_POINTER; // Return an appropriate error code
    }

    // Create an instance of the COM class
    hr = ::CoCreateInstance(m_clsid, nullptr, CLSCTX_LOCAL_SERVER, iid, reinterpret_cast<void**>(&pIUnknown));
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to create COM instance. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Query for the requested interface
    hr = pIUnknown->QueryInterface(iid, reinterpret_cast<void**>(ppIUnknown));
    if (FAILED(hr))
    {
        hr = S_FALSE;
        goto End;
    }

End:

    // Release the IUnknown pointer
    if (pIUnknown)
    {
        pIUnknown->Release();
        pIUnknown = nullptr;
    }

    return hr;
}

/// <summary>
/// Retrieves the IBigDriveConfiguration interface from the COM+ class instance.
/// </summary>
/// <param name="ppBigDriveConfiguration">A pointer to the IBigDriveConfiguration interface pointer to be populated.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveInterfaceProvider::GetIBigDriveConfiguration(IBigDriveConfiguration** ppBigDriveConfiguration)
{
    HRESULT hr = S_OK;

    if (ppBigDriveConfiguration == nullptr)
    {
        return E_POINTER; // Return an appropriate error code
    }

    // Get the IBigDriveConfiguration interface
    hr = GetInterface(IID_IBigDriveConfiguration, reinterpret_cast<IUnknown**>(ppBigDriveConfiguration));
    switch (hr)
    {
    case S_OK:
        // Successfully retrieved the interface
        break;
    case S_FALSE:
        WriteError(L"Doesn't implement IBigDriveConfiguration");
        hr = E_NOINTERFACE;
        break;
    default:
        s_eventLogger.WriteErrorFormmated(L"Failed to get IBigDriveConfiguration interface. HRESULT: 0x%08X", hr);
        goto End;
    }

End:
    return hr;
}

/// <summary>
/// Retrieves the IBigDriveEnumerate interface from the COM+ class instance.
/// </summary>
/// <param name="ppBigDriveEnumerate">A pointer to the IBigDriveEnumerate interface pointer to be populated.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveInterfaceProvider::GetIBigDriveEnumerate(IBigDriveEnumerate** ppBigDriveEnumerate)
{
    HRESULT hr = S_OK;

    if (ppBigDriveEnumerate == nullptr)
    {
        return E_POINTER; // Return an appropriate error code
    }

    // Get the IBigDriveEnumerate interface
    hr = GetInterface(IID_IBigDriveEnumerate, reinterpret_cast<IUnknown**>(ppBigDriveEnumerate));
    switch (hr)
    {
    case S_OK:
        // Successfully retrieved the interface
        break;
    case S_FALSE:
        WriteError(L"Doesn't implement IBigEnumerate");
        hr = E_NOINTERFACE;
        break;
    default:
        s_eventLogger.WriteErrorFormmated(L"Failed to get IBigEnumerate interface. HRESULT: 0x%08X", hr);
        goto End;
    }

End:
    return hr;
}

/// <summary>
/// Retrieves the IBigDriveFileInfo interface from the COM+ class instance.
/// </summary>
/// <param name="ppBigDriveFileInfo">A pointer to the IBigDriveFileInfo interface pointer to be populated.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveInterfaceProvider::GetIBigDriveFileInfo(IBigDriveFileInfo** ppBigDriveFileInfo)
{
    HRESULT hr = S_OK;

    if (ppBigDriveFileInfo == nullptr)
    {
        return E_POINTER; // Return an appropriate error code
    }

    // Get the IBigDriveFileInfo interface
    hr = GetInterface(IID_IBigDriveFileInfo, reinterpret_cast<IUnknown**>(ppBigDriveFileInfo));
    switch (hr)
    {
    case S_OK:
        // Successfully retrieved the interface
        break;
    case S_FALSE:
        WriteError(L"Doesn't implement IBigDriveFileInfo");
        hr = E_NOINTERFACE;
        break;
    default:
        s_eventLogger.WriteErrorFormmated(L"Failed to get IBigDriveFileInfo interface. HRESULT: 0x%08X", hr);
        goto End;
    }

End:
    return hr;
}



/// <summary>
/// Logs an error message with the CLSID of the provider.
/// </summary>
/// <param name="message">The error message to log.</param>
/// <returns>HRESULT indicating success or failure of the logging operation.</returns>
HRESULT BigDriveInterfaceProvider::WriteError(LPCWSTR message)
{
    return s_eventLogger.WriteErrorFormmated(
        L"Provider: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X} %s",
        m_clsid.Data1,
        m_clsid.Data2,
        m_clsid.Data3,
        m_clsid.Data4[0], m_clsid.Data4[1],
        m_clsid.Data4[2], m_clsid.Data4[3], m_clsid.Data4[4], m_clsid.Data4[5], m_clsid.Data4[6], m_clsid.Data4[7],
        message);
}

/// <summary>
/// Logs a formatted error message with the CLSID of the provider.
/// </summary>
/// <param name="formatter">The format string for the error message.</param>
/// <param name="...">The arguments for the format string.</param>
/// <returns>HRESULT indicating success or failure of the logging operation.</returns>
HRESULT BigDriveInterfaceProvider::WriteErrorFormmated(LPCWSTR formatter, ...)
{
    va_list args;

    va_start(args, formatter);
    wchar_t buffer[1024];
    ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);
    va_end(args);

    // Format the GUID components
    wchar_t guidBuffer[128];
    swprintf(
        guidBuffer,
        sizeof(guidBuffer) / sizeof(wchar_t),
        L"Provider: {%08lX-%04X-%04X-%02X%02X-%02X%02X-%02X%02X-%02X%02X} ",
        m_clsid.Data1,
        m_clsid.Data2,
        m_clsid.Data3,
        m_clsid.Data4[0], m_clsid.Data4[1],
        m_clsid.Data4[2], m_clsid.Data4[3], m_clsid.Data4[4], m_clsid.Data4[5], m_clsid.Data4[6], m_clsid.Data4[7]
    );

    // Prepend the GUID to the error message
    wchar_t finalBuffer[1024];
    swprintf(
        finalBuffer,
        sizeof(finalBuffer) / sizeof(wchar_t),
        L"%s%s",
        guidBuffer,
        buffer
    );

    return s_eventLogger.WriteError(finalBuffer);
}
