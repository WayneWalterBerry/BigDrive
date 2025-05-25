// <copyright file="BigDriveShellFolderTraceLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// Provider GUID for BigDriveShellFolderTraceLogger: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}

#pragma once

#include <TraceLoggingProvider.h>
#include <shlobj.h> // For IShellFolder and related interfaces

/// <summary>
/// Provides static methods for trace logging events related to the BigDrive Shell Folder.
/// </summary>
class BigDriveShellFolderTraceLogger
{
private:

    static __declspec(thread) LARGE_INTEGER s_startTime;

public:
    /// <summary>
    /// Registers the trace logging provider.
    /// </summary>
    static void Initialize();

    /// <summary>
    /// Unregisters the trace logging provider.
    /// </summary>
    static void Uninitialize();

    /// <summary>
    /// Logs a custom event message.
    /// </summary>
    /// <param name="message">The message to log.</param>
    static void LogEvent(const char* message);

    /// <summary>
    /// Logs entry into a function.
    /// </summary>
    /// <param name="functionName">The name of the function being entered.</param>
    static void LogEnter(LPCSTR functionName);

    static void LogExit(LPCSTR functionName, HRESULT hr);

    static void LogCreateInstance(LPCSTR functionName, REFIID refiid);

    static void LogParseDisplayName(LPCSTR functionName, LPOLESTR pszDisplayName);

    static void LogBindToObject(LPCSTR functionName, PCUIDLIST_RELATIVE pidl);

private:

    /// <summary>
    /// Stores the current time in thread-local storage for duration tracking.
    /// </summary>
    static void StoreCurrentTimeForDurationTracking();

    /// <summary>
    /// Gets the delta in seconds between the stored time and the current time.
    /// </summary>
    /// <returns>Elapsed time in seconds as a double.</returns>
    static double GetElapsedSecondsSinceStoredTime();

    /// <summary>
    /// Attempts to map a well-known shell IID to its common name as a BSTR.
    /// If the IID is recognized, bstrIIDName is set to a SysAllocString of the name and S_OK is returned.
    /// If the IID is not recognized, bstrIIDName is set to nullptr and S_FALSE is returned.
    /// The caller is responsible for freeing bstrIIDName with ::SysFreeString.
    /// </summary>
    /// <param name="riid">The IID to look up.</param>
    /// <param name="bstrIIDName">[out] Receives the allocated BSTR with the IID name, or nullptr if not found.</param>
    /// <returns>S_OK if found, S_FALSE if not found.</returns>
    static HRESULT GetShellIIDName(REFIID riid, BSTR& bstrIIDName);
};