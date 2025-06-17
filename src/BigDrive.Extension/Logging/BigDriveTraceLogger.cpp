// <copyright file="BigDriveTraceLogger.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveTraceLogger.h"

#include <shlguid.h>      // For PSGUIDs
#include <combaseapi.h>   // For StringFromGUID2
#include <wchar.h>

// Provider Id: {FE595A38-23DD-434D-BBCE-EAB7CA88C40F}
TRACELOGGING_DEFINE_PROVIDER(
	g_hBigDriveTraceProvider,
	"BigDrive.ShellFolder",
	(0xfe595a38, 0x23dd, 0x434d, 0xbb, 0xce, 0xea, 0xb7, 0xca, 0x88, 0xc4, 0x0f)
);

__declspec(thread) LARGE_INTEGER BigDriveTraceLogger::s_startTime = { 0 };

/// <inheritdoc />
void BigDriveTraceLogger::Initialize()
{
	TraceLoggingRegister(g_hBigDriveTraceProvider);
}

/// <inheritdoc />
void BigDriveTraceLogger::Uninitialize()
{
	TraceLoggingUnregister(g_hBigDriveTraceProvider);
}

/// <inheritdoc />
void BigDriveTraceLogger::LogEvent(const wchar_t* message)
{
	TraceLoggingWrite(g_hBigDriveTraceProvider, "BigDriveExtensionEvent", TraceLoggingWideString(message, "Message"));
}

/// <inheritdoc />
void BigDriveTraceLogger::LogEvent(LPCSTR functionName, const wchar_t* message)
{
	TraceLoggingWrite(g_hBigDriveTraceProvider, "BigDriveExtensionEvent", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(message, "Message"));
}

void BigDriveTraceLogger::LogEventFormatted(LPCSTR functionName, LPCWSTR formatter, ...)
{
	va_list args;

	va_start(args, formatter);
	wchar_t message[1024];
	::vswprintf(message, sizeof(message) / sizeof(message[0]), formatter, args);
	va_end(args);

	TraceLoggingWrite(g_hBigDriveTraceProvider, "BigDriveExtensionEvent", TraceLoggingValue(functionName, "FunctionName"), TraceLoggingValue(message, "Message"));
}

/// <inheritdoc />
void BigDriveTraceLogger::LogEnter(LPCSTR functionName)
{
	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();
	TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"));
}

/// <inheritdoc />
void BigDriveTraceLogger::LogEnter(LPCSTR functionName, REFIID refiid)
{
	HRESULT hr = S_OK;
	BSTR bstrIIDName = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	hr = BigDriveTraceLogger::GetShellIIDName(refiid, bstrIIDName);
	switch (hr)
	{
	case S_OK:
		TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingWideString(bstrIIDName, "IID"));
		::SysFreeString(bstrIIDName);
		break;
	default:
		break;
	}

	return;
}

/// <inheritdoc />
void BigDriveTraceLogger::LogEnter(LPCSTR functionName, REFCLSID clsid, REFIID riid)
{
	HRESULT hr = S_OK;
	BSTR bstrIIDName = nullptr;

	BigDriveTraceLogger::StoreCurrentTimeForDurationTracking();

	hr = BigDriveTraceLogger::GetShellIIDName(riid, bstrIIDName);
	switch (hr)
	{
	case S_OK:
		TraceLoggingWrite(g_hBigDriveTraceProvider, "Enter", TraceLoggingString(functionName, "FunctionName"), TraceLoggingGuid(clsid, "CLSID"), TraceLoggingWideString(bstrIIDName, "IID"));
		::SysFreeString(bstrIIDName);
		break;
	default:
		break;
	}
}

/// <inheritdoc />
void BigDriveTraceLogger::LogExit(LPCSTR functionName, HRESULT hr)
{
	double elapsedMillisSeconds = BigDriveTraceLogger::GetElapsedSecondsSinceStoredTime() * 1000.0; // Convert to milliseconds

	TraceLoggingWrite(
		g_hBigDriveTraceProvider,
		"Exit",
		TraceLoggingString(functionName, "FunctionName"),
		TraceLoggingValue(elapsedMillisSeconds, "Milliseconds"),
		TraceLoggingHexUInt32(hr, "HRESULT"));
}


/// <inheritdoc />
void BigDriveTraceLogger::StoreCurrentTimeForDurationTracking()
{
	::QueryPerformanceCounter(&s_startTime);
}

/// <inheritdoc />
double BigDriveTraceLogger::GetElapsedSecondsSinceStoredTime()
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
HRESULT BigDriveTraceLogger::GetShellIIDName(REFIID riid, BSTR& bstrIIDName)
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
	else if (IsEqualIID(riid, IID_IMarshal))
		bstrIIDName = ::SysAllocString(L"IID_IMarshal");
	else if (IsEqualIID(riid, IID_IMarshal2))
		bstrIIDName = ::SysAllocString(L"IID_IMarshal2");
	else if (IsEqualIID(riid, IID_IStdMarshalInfo))
		bstrIIDName = ::SysAllocString(L"IID_IStdMarshalInfo");
	else if (IsEqualIID(riid, IID_IExternalConnection))
		bstrIIDName = ::SysAllocString(L"IID_IExternalConnection");
	else if (IsEqualIID(riid, IID_IProvideClassInfo))
		bstrIIDName = ::SysAllocString(L"IID_IProvideClassInfo");
	else if (IsEqualIID(riid, IID_IShellFolder2))
		bstrIIDName = ::SysAllocString(L"IID_IShellFolder2");
	else if (IsEqualIID(riid, IID_IStream))
		bstrIIDName = ::SysAllocString(L"IID_IStream");
	else if (IsEqualIID(riid, IID_ITransferSource))
		bstrIIDName = ::SysAllocString(L"IID_ITransferSource");
	else if (IsEqualIID(riid, IID_ICallFactory))
		bstrIIDName = ::SysAllocString(L"IID_ICallFactory");
	else if (IsEqualIID(riid, IID_IDropSource))
		bstrIIDName = ::SysAllocString(L"IID_IDropSource");
	else if (IsEqualIID(riid, IID_IThumbnailHandlerFactory))
		bstrIIDName = ::SysAllocString(L"IID_IThumbnailHandlerFactory");
	else if (IsEqualIID(riid, IID_IExplorerPaneVisibility))
		bstrIIDName = ::SysAllocString(L"IID_IExplorerPaneVisibility");
	else if (IsEqualIID(riid, IID_IPreviewItem))
		bstrIIDName = ::SysAllocString(L"IID_IPreviewItem");


	if (bstrIIDName != nullptr)
		return S_OK;

	// Not found: convert riid to string
	LPOLESTR str = nullptr;
	HRESULT hr = ::StringFromIID(riid, &str);
	if (SUCCEEDED(hr) && str != nullptr)
	{
		bstrIIDName = ::SysAllocString(str);
		::CoTaskMemFree(str);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT BigDriveTraceLogger::GetFMTIDName(REFGUID guid, BSTR& bstrPSGUID)
{
	bstrPSGUID = nullptr;

	// Common property set GUIDs (PSGUIDs)
	if (IsEqualGUID(guid, FMTID_SummaryInformation))
		bstrPSGUID = ::SysAllocString(L"FMTID_SummaryInformation");
	else if (IsEqualGUID(guid, FMTID_DocSummaryInformation))
		bstrPSGUID = ::SysAllocString(L"FMTID_DocSummaryInformation");
	else if (IsEqualGUID(guid, FMTID_UserDefinedProperties))
		bstrPSGUID = ::SysAllocString(L"FMTID_UserDefinedProperties");
	else if (IsEqualGUID(guid, FMTID_DiscardableInformation))
		bstrPSGUID = ::SysAllocString(L"FMTID_DiscardableInformation");
	else if (IsEqualGUID(guid, FMTID_AudioSummaryInformation))
		bstrPSGUID = ::SysAllocString(L"FMTID_AudioSummaryInformation");
	else if (IsEqualGUID(guid, FMTID_VideoSummaryInformation))
		bstrPSGUID = ::SysAllocString(L"FMTID_VideoSummaryInformation");
	else if (IsEqualGUID(guid, FMTID_MediaFileSummaryInformation))
		bstrPSGUID = ::SysAllocString(L"FMTID_MediaFileSummaryInformation");
	else if (IsEqualGUID(guid, FMTID_DRM))
		bstrPSGUID = ::SysAllocString(L"FMTID_DRM");
	else if (IsEqualGUID(guid, FMTID_Storage))
		bstrPSGUID = ::SysAllocString(L"FMTID_Storage");
	else if (IsEqualGUID(guid, FMTID_Briefcase))
		bstrPSGUID = ::SysAllocString(L"FMTID_Briefcase");
	else if (IsEqualGUID(guid, FMTID_WebView))
		bstrPSGUID = ::SysAllocString(L"FMTID_WebView");
	else if (IsEqualGUID(guid, FMTID_ShellDetails))
		bstrPSGUID = ::SysAllocString(L"FMTID_ShellDetails");
	else if (IsEqualGUID(guid, FMTID_Query))
		bstrPSGUID = ::SysAllocString(L"FMTID_Query");
	else if (IsEqualGUID(guid, PSGUID_SUMMARYINFORMATION))
		bstrPSGUID = ::SysAllocString(L"PSGUID_SUMMARYINFORMATION");
	else if (IsEqualGUID(guid, PSGUID_DOCUMENTSUMMARYINFORMATION))
		bstrPSGUID = ::SysAllocString(L"PSGUID_DOCUMENTSUMMARYINFORMATION");
	else if (IsEqualGUID(guid, PSGUID_MUSIC))
		bstrPSGUID = ::SysAllocString(L"PSGUID_MUSIC");
	else if (IsEqualGUID(guid, PSGUID_DRM))
		bstrPSGUID = ::SysAllocString(L"PSGUID_DRM");
	else if (IsEqualGUID(guid, PSGUID_VIDEO))
		bstrPSGUID = ::SysAllocString(L"PSGUID_VIDEO");
	else if (IsEqualGUID(guid, PSGUID_AUDIO))
		bstrPSGUID = ::SysAllocString(L"PSGUID_AUDIO");
	else if (IsEqualGUID(guid, PSGUID_DISPLACED))
		bstrPSGUID = ::SysAllocString(L"PSGUID_DISPLACED");
	else if (IsEqualGUID(guid, PSGUID_BRIEFCASE))
		bstrPSGUID = ::SysAllocString(L"PSGUID_BRIEFCASE");
	else if (IsEqualGUID(guid, PSGUID_MISC))
		bstrPSGUID = ::SysAllocString(L"PSGUID_MISC");
	else if (IsEqualGUID(guid, PSGUID_WEBVIEW))
		bstrPSGUID = ::SysAllocString(L"PSGUID_WEBVIEW");
	else if (IsEqualGUID(guid, PSGUID_LINK))
		bstrPSGUID = ::SysAllocString(L"PSGUID_LINK");
	else if (IsEqualGUID(guid, PSGUID_STORAGE))
		bstrPSGUID = ::SysAllocString(L"PSGUID_STORAGE");

	if (bstrPSGUID != nullptr)
		return S_OK;

	// Fallback: return GUID as string
	WCHAR wszGuid[64] = { 0 };
	int cch = ::StringFromGUID2(guid, wszGuid, ARRAYSIZE(wszGuid));
	if (cch > 0)
	{
		bstrPSGUID = ::SysAllocString(wszGuid);
		return S_OK;
	}

	return S_FALSE;
}