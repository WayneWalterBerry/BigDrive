// <copyright file="BigDriveShellFolderTraceLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderTraceLogger.h"

#include "BigDriveTraceLogger.h"
#include "..\BigDriveShellFolder.h"

__declspec(thread) LARGE_INTEGER BigDriveShellFolderTraceLogger::s_startTime = { 0 };

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::Initialize(CLSID driveGuid)
{
	m_driveGuid = driveGuid;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::Uninitialize()
{
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEvent(const char* message)
{
	TraceLoggingWrite(g_hBigDriveTraceProvider, "BigDriveShellFolderEvent", TraceLoggingValue(message, "Message"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName)
{
	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();
	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, UINT iColumn)
{
	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();
	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingValue(iColumn, "Column"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl)
{
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	hr = BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl, bstrPath);
	if (SUCCEEDED(hr))
	{
		TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"));
		goto End;
	}

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));

End:

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl, UINT iColumn)
{
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	hr = BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl, bstrPath);
	if (SUCCEEDED(hr))
	{
		TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"), TraceLoggingValue(iColumn, "Column"));
		goto End;
	}

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingValue(iColumn, "Column"));

End:

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, LPCITEMIDLIST pidl, const SHCOLUMNID* pscid)
{
	BSTR bstrPath = nullptr;
	BSTR bstrFMTID = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl, bstrPath);
	BigDriveTraceLogger::GetFMTIDName(pscid->fmtid, bstrFMTID);

	TraceLoggingWrite(g_hBigDriveTraceProvider,
		"Enter",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingWideString(bstrPath, "Path"),
		TraceLoggingWideString(bstrFMTID, "fmtid"),
		TraceLoggingValue(pscid->pid, "pid"));

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	if (bstrFMTID != nullptr)
	{
		::SysFreeString(bstrFMTID);
		bstrFMTID = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogResults(LPCSTR functionName, IEnumIDList* pEnumIdList)
{
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;
	LPITEMIDLIST pidl = nullptr;
	ULONG fetched = 0;

	if (pEnumIdList == nullptr)
	{
		TraceLoggingWrite(g_hBigDriveTraceProvider, "Result", TraceLoggingString(functionName, "FunctionName"), TraceLoggingString("Invalid enumerator", "Error"));
		return;
	}

	// TODO This is Bad Form To Reset the Input Enumerator, it would be nice to iterate through it without resetting it.
	pEnumIdList->Reset();

	while (pEnumIdList->Next(1, &pidl, &fetched) == S_OK && fetched == 1)
	{
		hr = BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl, bstrPath);
		if (SUCCEEDED(hr))
		{
			TraceLoggingWrite(g_hBigDriveTraceProvider, "Result", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"));
		}
		else
		{
			TraceLoggingWrite(g_hBigDriveTraceProvider, "Result", TraceLoggingString(functionName, "FunctionName"));
		}

		if (bstrPath != nullptr)
		{
			::SysFreeString(bstrPath);
			bstrPath = nullptr;
		}

		if (pidl != nullptr)
		{
			::CoTaskMemFree(pidl);
			pidl = nullptr;
		}
	}

	pEnumIdList->Reset();

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	if (pidl != nullptr)
	{
		::CoTaskMemFree(pidl);
		pidl = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFIID riid, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	BSTR bstrPath = nullptr;
	BSTR bstrIIDName = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	LPCITEMIDLIST pidlCombine = ::ILCombine(pidl1, pidl2);

	BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidlCombine, bstrPath);
	BigDriveTraceLogger::GetShellIIDName(riid, bstrIIDName);

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingWideString(bstrIIDName, "IID"),
		TraceLoggingWideString(bstrPath, "Path"));

	if (pidlCombine != nullptr)
	{
		::CoTaskMemFree((LPVOID)pidlCombine);
		pidlCombine = nullptr;
	}

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	if (bstrIIDName)
	{
		::SysFreeString(bstrIIDName);
		bstrIIDName = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFIID riid, LPCITEMIDLIST pidl)
{
	BSTR bstrPath = nullptr;
	BSTR bstrIIDName = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl, bstrPath);
	BigDriveTraceLogger::GetShellIIDName(riid, bstrIIDName);

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrIIDName, "IID"), TraceLoggingWideString(bstrPath, "Path"));

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	if (bstrIIDName != nullptr)
	{
		::SysFreeString(bstrIIDName);
		bstrIIDName = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	BSTR bstrPath1 = nullptr;
	BSTR bstrPath2 = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	hr1 = BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl1, bstrPath1);
	hr2 = BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl2, bstrPath2);

	if (SUCCEEDED(hr1) && SUCCEEDED(hr2) && (bstrPath1 != nullptr) && (bstrPath2 != nullptr))
	{
		TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter",
			TraceLoggingString(functionName, "FunctionName"),
			TraceLoggingWideString(bstrPath1, "Path1"),
			TraceLoggingWideString(bstrPath2, "Path2"));
		goto End;
	}

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));

End:

	if (bstrPath1 != nullptr)
	{
		::SysFreeString(bstrPath1);
		bstrPath1 = nullptr;
	}

	if (bstrPath2 != nullptr)
	{
		::SysFreeString(bstrPath2);
		bstrPath2 = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, UINT cidl, PCUITEMID_CHILD_ARRAY apidl)
{
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;

	for (UINT i = 0; i < cidl; ++i)
	{
		PCUITEMID_CHILD pidl = apidl[i];

		hr = BigDriveShellFolder::GetPathForLogging(m_driveGuid, apidl[i], bstrPath);
		if (SUCCEEDED(hr))
		{
			TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"));
		}
		else
		{
			TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));

		}

		if (bstrPath != nullptr)
		{
			::SysFreeString(bstrPath);
			bstrPath = nullptr;
		}
	}
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, CLSID* pClassID)
{
	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();
	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(*pClassID, "CLSID"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFCLSID clsid, REFIID riid)
{
	HRESULT hr = S_OK;
	BSTR bstrIIDName = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	BigDriveTraceLogger::GetShellIIDName(riid, bstrIIDName);

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingGuid(clsid, "CLSID"),
		TraceLoggingWideString(bstrIIDName, "IID"));

	if (bstrIIDName != nullptr)
	{
		::SysFreeString(bstrIIDName);
		bstrIIDName = nullptr;
	}
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFIID refiid)
{
	BSTR bstrIIDName = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();
	BigDriveTraceLogger::GetShellIIDName(refiid, bstrIIDName);

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrIIDName, "IID"));

	if (bstrIIDName != nullptr)
	{
		::SysFreeString(bstrIIDName);
		bstrIIDName = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogParseDisplayName(LPCSTR functionName, LPOLESTR pszDisplayName)
{
	HRESULT hr = S_OK;
	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(pszDisplayName, "DisplayName"));
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogExit(LPCSTR functionName, HRESULT hr)
{
	double elapsedMillisSeconds = BigDriveTraceLogger::GetElapsedSecondsSinceStoredTime() * 1000.0; // Convert to milliseconds

	TraceLoggingWrite(
		g_hBigDriveTraceProvider,
		"Exit",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingValue(elapsedMillisSeconds, "Milliseconds"),
		TraceLoggingHexUInt32(hr, "HRESULT"));
}

void BigDriveShellFolderTraceLogger::LogExit(LPCSTR functionName, LPCITEMIDLIST pidl, HRESULT hr)
{
	BSTR bstrPath = nullptr;

	double elapsedMillisSeconds = BigDriveTraceLogger::GetElapsedSecondsSinceStoredTime() * 1000.0; // Convert to milliseconds

	HRESULT hrInternal = BigDriveShellFolder::GetPathForLogging(m_driveGuid, pidl, bstrPath);
	if (SUCCEEDED(hrInternal))
	{
		TraceLoggingWrite(g_hBigDriveTraceProvider,
			"Exit",
			TraceLoggingString(functionName, "FunctionName"),
			TraceLoggingWideString(bstrPath, "Path"),
			TraceLoggingValue(elapsedMillisSeconds, "Milliseconds"),
			TraceLoggingHexUInt32(hr, "HRESULT"));

		goto End;
	}

	TraceLoggingWrite(
		g_hBigDriveTraceProvider,
		"Exit",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingValue(elapsedMillisSeconds, "Milliseconds"),
		TraceLoggingHexUInt32(hr, "HRESULT"));

End:

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	return;
}
