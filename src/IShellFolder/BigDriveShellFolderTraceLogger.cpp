// <copyright file="BigDriveShellFolderTraceLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderTraceLogger.h"
#include "ILExtensions.h"

// Provider Id: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}
TRACELOGGING_DEFINE_PROVIDER(
    g_hMyProvider,
    "BigDrive.ShellFolder",
    (0xa356d4cc, 0xcdac, 0x4894, 0xa9, 0x3d, 0x35, 0xc4, 0xc3, 0xf8, 0x49, 0x44)
);

__declspec(thread) LARGE_INTEGER BigDriveShellFolderTraceLogger::s_startTime = { 0 };

/// <summary>
/// Registers the trace logging provider.
/// </summary>
void BigDriveShellFolderTraceLogger::Initialize()
{
    TraceLoggingRegister(g_hMyProvider);
}

/// <summary>
/// Unregisters the trace logging provider.
/// </summary>
void BigDriveShellFolderTraceLogger::Uninitialize()
{
    TraceLoggingUnregister(g_hMyProvider);
}

/// <summary>
/// Logs a custom event message.
/// </summary>
/// <param name="message">The message to log.</param>
void BigDriveShellFolderTraceLogger::LogEvent(const char* message)
{
    TraceLoggingWrite(g_hMyProvider, "BigDriveShellFolderEvent", TraceLoggingValue(message, "Message"));
}

/// <summary>
/// Logs entry into a function.
/// </summary>
/// <param name="functionName">The name of the function being entered.</param>
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName)
{
	StoreCurrentTimeForDurationTracking(); 
    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));
}

void BigDriveShellFolderTraceLogger::LogCreateInstance(LPCSTR functionName, REFIID refiid)
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

void BigDriveShellFolderTraceLogger::LogParseDisplayName(LPCSTR functionName, LPOLESTR pszDisplayName)
{
	HRESULT hr = S_OK;
    StoreCurrentTimeForDurationTracking();

    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(pszDisplayName, "DisplayName"));
}

void BigDriveShellFolderTraceLogger::LogBindToObject(LPCSTR functionName, PCUIDLIST_RELATIVE pidl)
{
    HRESULT hr = S_OK;
	BSTR bstrPidl = nullptr;

    StoreCurrentTimeForDurationTracking();
	::ILSerialize(pidl, bstrPidl);

    TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPidl, "PIDL"));

	::SysFreeString(bstrPidl);
}

/// <summary>
/// Logs exit from a function.
/// </summary>
/// <param name="functionName">The name of the function being entered.</param>
/// <param name="hr"></param>
void BigDriveShellFolderTraceLogger::LogExit(LPCSTR functionName, HRESULT hr)
{
    double elapsedSeconds = GetElapsedSecondsSinceStoredTime();

    TraceLoggingWrite(
        g_hMyProvider,
        "Exit ",
        TraceLoggingString(functionName, "FunctionName"),
        TraceLoggingValue(elapsedSeconds, "ElapsedSeconds"),
        TraceLoggingHexUInt32(hr, "HRESULT")
    );
}

/// <summary>
/// Stores the current time in thread-local storage for duration tracking.
/// </summary>
void BigDriveShellFolderTraceLogger::StoreCurrentTimeForDurationTracking()
{
    ::QueryPerformanceCounter(&s_startTime);
}

/// <summary>
/// Gets the delta in seconds between the stored time and the current time.
/// </summary>
/// <returns>Elapsed time in seconds as a double.</returns>
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

/// <summary>
/// Attempts to map a well-known shell IID to its common name as a BSTR.
/// If the IID is recognized, bstrIIDName is set to a SysAllocString of the name and S_OK is returned.
/// If the IID is not recognized, bstrIIDName is set to nullptr and S_FALSE is returned.
/// The caller is responsible for freeing bstrIIDName with ::SysFreeString.
/// </summary>
/// <param name="riid">The IID to look up.</param>
/// <param name="bstrIIDName">[out] Receives the allocated BSTR with the IID name, or nullptr if not found.</param>
/// <returns>S_OK if found, S_FALSE if not found.</returns>
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

    if (bstrIIDName != nullptr)
        return S_OK;
    else
        return S_FALSE;
}