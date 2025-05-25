// <copyright file="BigDriveShellFolderTraceLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

// Provider GUID for BigDriveShellFolderTraceLogger: {A356D4CC-CDAC-4894-A93D-35C4C3F84944}

#pragma once

#include <TraceLoggingProvider.h>
#include <shlobj.h> 

/// <summary>
/// Provides static, thread-safe methods for trace logging events and diagnostics related to the BigDrive Shell Folder.
/// Supports logging function entry/exit, method parameters, and duration tracking for performance analysis.
/// </summary>
class BigDriveShellFolderTraceLogger
{
private:

    /// <summary>
    /// Thread-local storage for high-resolution timing, used to track method durations.
    /// </summary>
    static __declspec(thread) LARGE_INTEGER s_startTime;

public:
    /// <summary>
    /// Registers the trace logging provider for the BigDrive Shell Folder.
    /// Call this before any logging is performed.
    /// </summary>
    static void Initialize();

    /// <summary>
    /// Unregisters the trace logging provider.
    /// Call this during shutdown or cleanup to release resources.
    /// </summary>
    static void Uninitialize();

    /// <summary>
    /// Logs a custom informational event message.
    /// </summary>
    /// <param name="message">The message to log.</param>
    static void LogEvent(const char* message);

    /// <summary>
    /// Logs entry into a function, and stores the current time for duration tracking.
    /// </summary>
    /// <param name="functionName">The name of the function being entered.</param>
    static void LogEnter(LPCSTR functionName);

    /// <summary>
    /// Logs exit from a function, including the elapsed time since LogEnter and the HRESULT result.
    /// </summary>
    /// <param name="functionName">The name of the function being exited.</param>
    /// <param name="hr">The HRESULT returned by the function.</param>
    static void LogExit(LPCSTR functionName, HRESULT hr);

    /// <summary>
    /// Logs entry into DllGetClassObject, including the CLSID and IID parameters.
    /// </summary>
    /// <param name="functionName">The function name (typically __FUNCTION__).</param>
    /// <param name="clsid">The CLSID requested.</param>
    /// <param name="riid">The IID requested.</param>
    static void LogDllGetClassObject(LPCSTR functionName, REFCLSID clsid, REFIID riid);

    /// <summary>
    /// Logs entry into CreateInstance, including the requested IID.
    /// </summary>
    /// <param name="functionName">The function name (typically __FUNCTION__).</param>
    /// <param name="refiid">The IID requested for the new instance.</param>
    static void LogCreateInstance(LPCSTR functionName, REFIID refiid);

    /// <summary>
    /// Logs entry into ParseDisplayName, including the display name parameter.
    /// </summary>
    /// <param name="functionName">The function name (typically __FUNCTION__).</param>
    /// <param name="pszDisplayName">The display name being parsed.</param>
    static void LogParseDisplayName(LPCSTR functionName, LPOLESTR pszDisplayName);

    /// <summary>
    /// Logs entry into BindToObject, including a serialized representation of the PIDL.
    /// </summary>
    /// <param name="functionName">The function name (typically __FUNCTION__).</param>
    /// <param name="pidl">The PIDL being bound to.</param>
    static void LogBindToObject(LPCSTR functionName, PCUIDLIST_RELATIVE pidl);

    /// <summary>
    /// Logs entry into GetDisplayNameOf, including a serialized representation of the PIDL.
    /// </summary>
    /// <param name="functionName">The function name (typically __FUNCTION__).</param>
    /// <param name="pidl">The PIDL whose display name is being retrieved.</param>
    static void LogGetDisplayNameOf(LPCSTR functionName, PCUIDLIST_RELATIVE pidl);

private:

    /// <summary>
    /// Stores the current high-resolution time in thread-local storage for duration tracking.
    /// Called at function entry to enable later measurement of elapsed time.
    /// </summary>
    static void StoreCurrentTimeForDurationTracking();

    /// <summary>
    /// Gets the elapsed time in seconds since the last call to StoreCurrentTimeForDurationTracking on the current thread.
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