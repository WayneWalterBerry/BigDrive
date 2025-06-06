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
	/// The Drive guid 
	/// </summary>
	CLSID m_driveGuid;

	/// <summary>
	/// Thread-local storage for high-resolution timing, used to track method durations.
	/// </summary>
	static __declspec(thread) LARGE_INTEGER s_startTime;

public:

	/// <summary>
	/// Registers the trace logging provider for the BigDrive Shell Folder.
	/// Call this before any logging is performed.
	/// </summary>
	void Initialize(CLSID driveGuid);

	/// <summary>
	/// Unregisters the trace logging provider.
	/// Call this during shutdown or cleanup to release resources.
	/// </summary>
	void Uninitialize();

	/// <summary>
	/// Logs a custom informational event message.
	/// </summary>
	/// <param name="message">The message to log.</param>
	void LogEvent(const char* message);

	/// <summary>
	/// Logs entry into a function, and stores the current time for duration tracking.
	/// </summary>
	/// <param name="functionName">The name of the function being entered.</param>
	void LogEnter(LPCSTR functionName);

	void LogEnter(LPCSTR functionName, UINT iColumn);

	void LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl);

	void LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl, UINT iColumn);

	void LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl, const SHCOLUMNID* pscid);

	void LogEnter(LPCSTR functionName, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);

	void LogEnter(LPCSTR functionName, UINT cidl, PCUITEMID_CHILD_ARRAY apidl);

	void LogEnter(LPCSTR functionName, REFIID riid, LPCITEMIDLIST pidl);

	void LogEnter(LPCSTR functionName, REFIID riid, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

	void LogEnter(LPCSTR functionName, CLSID* pClassID);

	/// <summary>
	/// Logs entry into DllGetClassObject, including the CLSID and IID parameters.
	/// </summary>
	/// <param name="functionName">The function name (typically __FUNCTION__).</param>
	/// <param name="clsid">The CLSID requested.</param>
	/// <param name="riid">The IID requested.</param>
	void LogEnter(LPCSTR functionName, REFCLSID clsid, REFIID riid);

	/// <summary>
	/// Logs entry into CreateInstance, including the requested IID.
	/// </summary>
	/// <param name="functionName">The function name (typically __FUNCTION__).</param>
	/// <param name="refiid">The IID requested for the new instance.</param>
	void LogEnter(LPCSTR functionName, REFIID refiid);

	/// <summary>
	/// Logs entry into ParseDisplayName, including the display name parameter.
	/// </summary>
	/// <param name="functionName">The function name (typically __FUNCTION__).</param>
	/// <param name="pszDisplayName">The display name being parsed.</param>
	void LogParseDisplayName(LPCSTR functionName, LPOLESTR pszDisplayName);

	/// <summary>
	/// Logs exit from a function, including the elapsed time since LogEnter and the HRESULT result.
	/// </summary>
	/// <param name="functionName">The name of the function being exited.</param>
	/// <param name="hr">The HRESULT returned by the function.</param>
	void LogExit(LPCSTR functionName, HRESULT hr);

	void LogExit(LPCSTR functionName, LPCITEMIDLIST pidl, HRESULT hr);

	void LogResults(LPCSTR functionName, IEnumIDList* pEnumIdList);
};