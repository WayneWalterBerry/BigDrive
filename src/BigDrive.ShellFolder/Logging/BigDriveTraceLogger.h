// <copyright file="BigDriveTraceLogger.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <TraceLoggingProvider.h>
#include <shlobj.h> 

/// <summary>
/// Hanldes the trace logging provider for the BigDrive Shell Folder.
/// </summary>
/// <param name=""></param>
TRACELOGGING_DECLARE_PROVIDER(g_hBigDriveTraceProvider);

/// <summary>
/// Provides static, thread-safe methods for trace logging events and diagnostics related to the BigDrive Shell Folder.
/// Supports logging function entry/exit, method parameters, and duration tracking for performance analysis.
/// </summary>
class BigDriveTraceLogger
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
	/// Logs entry into CreateInstance, including the requested IID.
	/// </summary>
	/// <param name="functionName">The function name (typically __FUNCTION__).</param>
	static void LogEnter(LPCSTR functionName);

	/// <summary>
	/// Logs entry into DllGetClassObject, including the CLSID and IID parameters.
	/// </summary>
	/// <param name="functionName">The function name (typically __FUNCTION__).</param>
	/// <param name="clsid">The CLSID requested.</param>
	/// <param name="riid">The IID requested.</param>
	static void LogEnter(LPCSTR functionName, REFCLSID clsid, REFIID riid);

	/// <summary>
	/// Logs entry into CreateInstance, including the requested IID.
	/// </summary>
	/// <param name="functionName">The function name (typically __FUNCTION__).</param>
	/// <param name="riid">The IID requested for the new instance.</param>
	static void LogEnter(LPCSTR functionName, REFIID riid);

	/// <summary>
	/// Logs exit from a function, including the elapsed time since LogEnter and the HRESULT result.
	/// </summary>
	/// <param name="functionName">The name of the function being exited.</param>
	/// <param name="hr">The HRESULT returned by the function.</param>
	static void LogExit(LPCSTR functionName, HRESULT hr);

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

	static HRESULT GetFMTIDName(REFGUID guid, BSTR& bstrPSGUID);

private:

	// Grant access to all private/protected members
	friend class BigDriveShellFolderTraceLogger;
};