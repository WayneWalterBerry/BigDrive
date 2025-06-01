// <copyright file="BigDriveShellFolderTraceLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderTraceLogger.h"
#include "BigDriveShellFolder.h"

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
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;

	StoreCurrentTimeForDurationTracking();

	hr = BigDriveShellFolder::GetPath(pidl, bstrPath);
	if (SUCCEEDED(hr))
	{
		TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"));
		goto End;
	}

	TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));

End:

	if (bstrPath != nullptr)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	return;
}

/// <inheritdoc />
void BigDriveShellFolderTraceLogger::LogResults(LPCSTR functionName, IEnumIDList *pEnumIdList)
{
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;
	LPITEMIDLIST pidl = nullptr;
	ULONG fetched = 0;

	if (pEnumIdList == nullptr)
	{
		TraceLoggingWrite(g_hMyProvider, "Result", TraceLoggingString(functionName, "FunctionName"), TraceLoggingString("Invalid enumerator", "Error"));
		return;
	}

	// TODO This is Bad Form To Reset the Input Enumerator, it would be nice to iterate through it without resetting it.
	pEnumIdList->Reset();

	while (pEnumIdList->Next(1, &pidl, &fetched) == S_OK && fetched == 1)
	{
		hr = BigDriveShellFolder::GetPath(pidl, 0, bstrPath);
		if (SUCCEEDED(hr))
		{
			TraceLoggingWrite(g_hMyProvider, "Result", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"));
		}
		else
		{
			TraceLoggingWrite(g_hMyProvider, "Result", TraceLoggingString(functionName, "FunctionName"));
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

void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, REFIID riid, LPCITEMIDLIST pidl)
{
	HRESULT hr = S_OK;
	BSTR bstrPath = nullptr;
	BSTR bstrIIDName = nullptr;

	StoreCurrentTimeForDurationTracking();

	hr = BigDriveShellFolder::GetPath(pidl, 0, bstrPath);
	if (SUCCEEDED(hr))
	{
		hr = GetShellIIDName(riid, bstrIIDName);
		switch (hr)
		{
		case S_OK:
			TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrIIDName, "IID"), TraceLoggingWideString(bstrPath, "Path"));
			::SysFreeString(bstrIIDName);
			break;
		case S_FALSE:
			TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(riid, "IID"), TraceLoggingWideString(bstrPath, "Path"));
			break;
		default:
			break;
		}

		goto End;
	}

	hr = GetShellIIDName(riid, bstrIIDName);
	switch (hr)
	{
	case S_OK:
		TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrIIDName, "IID"));
		::SysFreeString(bstrIIDName);
		break;
	case S_FALSE:
		TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(riid, "IID"));
		break;
	default:
		break;
	}

End:

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
void BigDriveShellFolderTraceLogger::LogEnter(LPCSTR functionName, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	BSTR bstrPath1 = nullptr;
	BSTR bstrPath2 = nullptr;

	StoreCurrentTimeForDurationTracking();

	hr1 = BigDriveShellFolder::GetPath(pidl1, 0, bstrPath1);
	hr2 = BigDriveShellFolder::GetPath(pidl2, 0, bstrPath2);

	if (SUCCEEDED(hr1) && SUCCEEDED(hr2) && (bstrPath1 != nullptr) && (bstrPath2 != nullptr))
	{
		TraceLoggingWrite(g_hMyProvider, "Enter",
			TraceLoggingString(functionName, "FunctionName"),
			TraceLoggingWideString(bstrPath1, "Path1"),
			TraceLoggingWideString(bstrPath2, "Path2"));
		goto End;
	}

	TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));

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

		hr = BigDriveShellFolder::GetPath(apidl[i], 0, bstrPath);
		if (SUCCEEDED(hr))
		{
			TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrPath, "Path"));
		}
		else
		{
			TraceLoggingWrite(g_hMyProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));

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
	double elapsedMillisSeconds = GetElapsedSecondsSinceStoredTime() * 1000.0; // Convert to milliseconds

	TraceLoggingWrite(
		g_hMyProvider,
		"Exit",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingValue(elapsedMillisSeconds, "Milliseconds"),
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