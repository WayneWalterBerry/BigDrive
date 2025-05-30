// <copyright file="BigDriveShellFolderTraceLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderTraceLogger.h"

// Provider Id: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}
TRACELOGGING_DEFINE_PROVIDER(
    g_hMyProvider,
    "BigDrive.ShellFolder",
    (0xa356d4cc, 0xcdac, 0x4894, 0xa9, 0x3d, 0x35, 0xc4, 0xc3, 0xf8, 0x49, 0x44)
);

__declspec(thread) LARGE_INTEGER BigDriveShellFolderTraceLogger::s_startTime = { 0 };

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::Initialize()
{
    TraceLoggingRegister(g_hMyProvider);
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::Uninitialize()
{
    TraceLoggingUnregister(g_hMyProvider);
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEvent(const char* message)
{
    TraceLoggingWrite(g_hMyProvider, "BigDriveShellFolderEvent", TraceLoggingValue(message, "Message"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName)
{
    StoreCurrentTimeForDurationTracking();
    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl)
{
    WCHAR szPath[MAX_PATH];

    StoreCurrentTimeForDurationTracking();

    if (::SHGetPathFromIDListW(pidl, szPath))
    {
        TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(szPath, "Path"));
    }

    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));
}


/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, CLSID* pClassID)
{
    StoreCurrentTimeForDurationTracking();
    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(*pClassID, "CLSID"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFCLSID clsid, REFIID riid)
{
    HRESULT hr = S_OK;
    BSTR bstrIIDName = nullptr;

    StoreCurrentTimeForDurationTracking();

    hr = GetShellIIDName(riid, bstrIIDName);
    switch (hr)
    {
    case S_OK:
        TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(clsid, "CLSID"), TraceLoggingWideString(bstrIIDName, "IID"));
        ::SysFreeString(bstrIIDName);
        break;
    case S_FALSE:
        TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(clsid, "CLSID"), TraceLoggingGuid(riid, "IID"));
        break;
    default:
        break;
    }
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFIID refiid)
{
    HRESULT hr = S_OK;
    BSTR bstrIIDName = nullptr;

    StoreCurrentTimeForDurationTracking();

    hr = GetShellIIDName(refiid, bstrIIDName);
    switch (hr)
    {
    case S_OK:
        TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrIIDName, "IID"));
        ::SysFreeString(bstrIIDName);
        break;
    case S_FALSE:
        TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(refiid, "IID"));
        break;
    default:
        break;
    }

    return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogParseDisplayName(LPCSTR functionName, LPOLESTR pszDisplayName)
{
    HRESULT hr = S_OK;
    StoreCurrentTimeForDurationTracking();

    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(pszDisplayName, "DisplayName"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogExit(LPCSTR functionName, HRESULT hr)
{
    double elapsedSeconds = GetElapsedSecondsSinceStoredTime();

    TraceLoggingWrite(
        g_hMyProvider,
        "Exit",
        TraceLoggingString(functionName, "FunctionName"),
        TraceLoggingValue(elapsedSeconds, "ElapsedSeconds"),
        TraceLoggingHexUInt32(hr, "HRESULT")
    );
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::StoreCurrentTimeForDurationTracking()
{
    ::QueryPerformanceCounter(&s_startTime);
}

/// <inheritdoc />
double BigDriveShellFolderTraceLogger::GetElapsedSecondsSinceStoredTime()
{
    LARGE_INTEGER now, freq;

    ::QueryPerformanceCounter(&now);
    ::QueryPerformanceFrequency(&freq);

    if (freq.QuadPart == 0)
    {
        return 0.0;
    }

    double result = (double)(now.QuadPart - s_startTime.QuadPart) / (double)freq.QuadPart;

    return result;
}

/// <inheritdoc />
HRESULT BigDriveShellFolderTraceLogger::GetShellIIDName(REFIID riid, BSTR& bstrIIDName)
{
    bstrIIDName = nullptr;
    if (IsEqualIID(riid, IID_IUnknown))
        bstrIIDName = ::SysAllocString(L"IID_IUnknown");
    else if (IsEqualIID(riid, IID_IShellFolder))
        bstrIIDName = ::SysAllocString(L"IID_IShellFolder");
    else if (IsEqualIID(riid, IID_IShellView))
        bstrIIDName = ::SysAllocString(L"IID_IShellView");
    else if (IsEqualIID(riid, IID_IPersist))
        bstrIIDName = ::SysAllocString(L"IID_IPersist");
    else if (IsEqualIID(riid, IID_IPersistFolder))
        bstrIIDName = ::SysAllocString(L"IID_IPersistFolder");
    else if (IsEqualIID(riid, IID_IPersistFolder2))
        bstrIIDName = ::SysAllocString(L"IID_IPersistFolder2");
    else if (IsEqualIID(riid, IID_IContextMenu))
        bstrIIDName = ::SysAllocString(L"IID_IContextMenu");
    else if (IsEqualIID(riid, IID_IDataObject))
        bstrIIDName = ::SysAllocString(L"IID_IDataObject");
    else if (IsEqualIID(riid, IID_IDropTarget))
        bstrIIDName = ::SysAllocString(L"IID_IDropTarget");
    else if (IsEqualIID(riid, IID_IExtractIconW))
        bstrIIDName = ::SysAllocString(L"IID_IExtractIconW");
    else if (IsEqualIID(riid, IID_IShellIcon))
        bstrIIDName = ::SysAllocString(L"IID_IShellIcon");
    else if (IsEqualIID(riid, IID_IShellDetails))
        bstrIIDName = ::SysAllocString(L"IID_IShellDetails");
    else if (IsEqualIID(riid, IID_IQueryInfo))
        bstrIIDName = ::SysAllocString(L"IID_IQueryInfo");
    else if (IsEqualIID(riid, IID_IEnumIDList))
        bstrIIDName = ::SysAllocString(L"IID_IEnumIDList");
    else if (IsEqualIID(riid, IID_IShellItem))
        bstrIIDName = ::SysAllocString(L"IID_IShellItem");
    else if (IsEqualIID(riid, IID_IShellItem2))
        bstrIIDName = ::SysAllocString(L"IID_IShellItem2");
    else if (IsEqualIID(riid, IID_IClassFactory))
        bstrIIDName = ::SysAllocString(L"IID_IClassFactory");
    else if (IsEqualIID(riid, IID_IObjectWithBackReferences))
        bstrIIDName = ::SysAllocString(L"IID_IObjectWithBackReferences");

    if (bstrIIDName != nullptr)
        return S_OK;
    else
        return S_FALSE;
}