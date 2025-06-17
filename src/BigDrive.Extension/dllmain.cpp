// <copyright file="dllmain.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "dllmain.h"
#include "LaunchDebugger.h"
#include "BigDriveExtensionClassFactory.h"
#include "Logging\BigDriveTraceLogger.h"
#include "RegistrationHelper.h"

#include <windows.h>
#include <strsafe.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

// {CBB26998-8B10-4599-8AB7-01AF65F3F68B}
extern "C" const CLSID CLSID_BigDriveExtension =
{ 0xcbb26998, 0x8b10, 0x4599, { 0x8a, 0xb7, 0x01, 0xaf, 0x65, 0xf3, 0xf6, 0x8b } };

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
	HRESULT hr = S_OK;
	bool bitMatch = FALSE;
	LPITEMIDLIST pidlMyComputer = nullptr;

	BigDriveTraceLogger::LogEnter(__FUNCTION__);

	hr = CheckDllAndOSBitnessMatch(bitMatch);
	if (FAILED(hr))
	{
		// Log the error and return failure.
		BigDriveTraceLogger::LogEvent(L"Failed to check DLL and OS bitness match. Ensure the extension is compatible with your system architecture.");
		goto End;
	}

	if (!bitMatch)
	{
		// Log a message indicating that the bitness of the DLL and OS do not match.
		BigDriveTraceLogger::LogEvent(L"DLL and OS bitness do not match. Please ensure you are using the correct version of the extension for your system architecture.");
		hr = E_FAIL;
		goto End;
	}

	hr = RegisterContextMenuExtension(CLSID_BigDriveExtension);
	if (FAILED(hr))
	{
		// Log the error and return failure.
		goto End;
	}

	hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
	if (SUCCEEDED(hr) && pidlMyComputer != nullptr)
	{
		BigDriveTraceLogger::LogEvent(L"Refreshing the Shell Folder");
		::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlMyComputer, NULL);
		::CoTaskMemFree(pidlMyComputer);
	}

End:

	BigDriveTraceLogger::LogExit(__FUNCTION__, hr);

	return hr;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
	return S_OK;
}

/// <summary>
/// Retrieves a class object from a DLL for the specified CLSID.
/// This function is typically implemented in a DLL that provides COM objects,
/// allowing clients to obtain an IShellFolder instance.
/// </summary>
/// <param name="rclsid">The CLSID of the object to retrieve.</param>
/// <param name="riid">The interface identifier (IID) for the requested interface.</param>
/// <param name="ppv">Pointer to the location where the interface pointer will be stored.</param>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	HRESULT hr = S_OK;
	BigDriveExtensionClassFactory* pFactory = nullptr;

	if (ppv == nullptr)
	{
		hr = E_POINTER;
		goto End;
	}

	*ppv = nullptr;

	if (::IsEqualCLSID(rclsid, CLSID_BigDriveExtension))
	{
		pFactory = new BigDriveExtensionClassFactory();
		if (!pFactory)
		{
			hr = E_OUTOFMEMORY;
			goto End;
		}

		hr = pFactory->QueryInterface(riid, ppv);
	}
	else
	{
		hr = CLASS_E_CLASSNOTAVAILABLE;
	}

End:

	if (pFactory)
	{
		pFactory->Release();
		pFactory = nullptr;
	}

	return hr;
}

/// <summary>
/// Registers the context menu handler for "My PC" (This PC) in the registry.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT RegisterMyPCContextMenuHandler()
{
	HRESULT hr = S_OK;
	HKEY hKeyShellEx = nullptr;
	HKEY hKeyDriveHandler = nullptr;
	HKEY hKeyContextMenuHandlers = nullptr;
	LONG lResult = 0;
	WCHAR szCLSID[64] = { 0 };
	size_t len = 0;

	// Convert CLSID_BigDriveExtension to string (with braces)
	if (FAILED(::StringFromGUID2(CLSID_BigDriveExtension, szCLSID, ARRAYSIZE(szCLSID))))
	{
		hr = E_FAIL;
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to convert CLSID_BigDriveExtension to string. HRESULT: 0x%08X", hr);
		goto End;
	}

	// 3. Register the handler under My PC ContextMenuHandlers

	lResult = ::RegCreateKeyExW(
		HKEY_CLASSES_ROOT,
		L"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shellex\\ContextMenuHandlers\\BigDriveExtension",
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY,
		nullptr,
		&hKeyDriveHandler,
		nullptr);

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to create registry key for My PC ContextMenuHandlers. Error: %u", lResult);
		goto End;
	}

	// Set the default value to the CLSID string (without braces)
	lResult = ::RegSetValueExW(
		hKeyDriveHandler,
		nullptr,
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(szCLSID),
		static_cast<DWORD>((::lstrlenW(szCLSID) + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to set default value for My PC ContextMenuHandlers. Error: %u", lResult);
		goto End;
	}

End:

	if (hKeyShellEx)
	{
		::RegCloseKey(hKeyShellEx);
		hKeyShellEx = nullptr;
	}

	if (hKeyDriveHandler)
	{
		::RegCloseKey(hKeyDriveHandler);
		hKeyDriveHandler = nullptr;
	}

	if (hKeyDriveHandler)
	{
		::RegCloseKey(hKeyDriveHandler);
		hKeyDriveHandler = nullptr;
	}

	return hr;

}

/// </ inheritdoc>
/// <inheritdoc>
HRESULT RegisterContextMenuExtension(const CLSID& clsidExtension)
{
	HRESULT hr = S_OK;
	HKEY hKeyCLSID = nullptr;
	HKEY hKeyInproc = nullptr;
	HKEY hKeyDriveHandler = nullptr;
	WCHAR szCLSID[64] = { 0 };
	WCHAR szCLSIDKey[128] = { 0 };
	WCHAR szModulePath[MAX_PATH] = { 0 };
	LONG lResult = 0;

	BigDriveTraceLogger::LogEnter(__FUNCTION__);

	// Convert CLSID to string
	if (FAILED(::StringFromGUID2(clsidExtension, szCLSID, ARRAYSIZE(szCLSID))))
	{
		hr = E_FAIL;
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to convert CLSID to string. HRESULT: 0x%08X", hr);
		goto End;
	}

	// Build CLSID key path: "CLSID\\{...}"
	hr = ::StringCchPrintfW(szCLSIDKey, ARRAYSIZE(szCLSIDKey), L"CLSID\\%s", szCLSID);
	if (FAILED(hr))
	{
		goto End;
	}

	// 1. Register the COM object under HKCR\CLSID\{CLSID_BigDriveExtension}
	lResult = ::RegCreateKeyExW(
		HKEY_CLASSES_ROOT,
		szCLSIDKey,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		nullptr,
		&hKeyCLSID,
		nullptr);

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to create registry key '%s'. Error: %u", szCLSIDKey, lResult);
		goto End;
	}

	// Set the default value (optional, but recommended)
	lResult = ::RegSetValueExW(
		hKeyCLSID,
		nullptr,
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(L"BigDrive Shell Context Menu Extension"),
		static_cast<DWORD>((::lstrlenW(L"BigDrive Shell Context Menu Extension") + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to set default value for CLSID key '%s'. Error: %u", szCLSIDKey, lResult);
		goto End;
	}

	// 2. Register InprocServer32 subkey
	lResult = ::RegCreateKeyExW(
		hKeyCLSID,
		L"InprocServer32",
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		nullptr,
		&hKeyInproc,
		nullptr);

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to create InprocServer32 key for CLSID '%s'. Error: %u", szCLSIDKey, lResult);
		goto End;
	}

	// Get the full path to this DLL
	if (!::GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), szModulePath, ARRAYSIZE(szModulePath)))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to get module file name. Error: %u", hr);
		goto End;
	}

	// Set default value to DLL path
	lResult = ::RegSetValueExW(
		hKeyInproc,
		nullptr,
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(szModulePath),
		static_cast<DWORD>((::lstrlenW(szModulePath) + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to set default value for InprocServer32 key. Error: %u", lResult);
		goto End;
	}

	// Set ThreadingModel to Apartment
	lResult = ::RegSetValueExW(
		hKeyInproc,
		L"ThreadingModel",
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(L"Apartment"),
		static_cast<DWORD>((::lstrlenW(L"Apartment") + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		BigDriveTraceLogger::LogEventFormatted(__FUNCTION__, L"Failed to set ThreadingModel for InprocServer32 key. Error: %u", lResult);
		goto End;
	}

	hr = TakeOwnershipAndGrantFullControl(L"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}", &RegisterMyPCContextMenuHandler);
	if (FAILED(hr))
	{
		BigDriveTraceLogger::LogEvent(L"RegisterShellFolder: Failed to take ownership and grant full control.");
		goto End;
	}

End:

	BigDriveTraceLogger::LogExit(__FUNCTION__, hr);

	if (hKeyInproc)
	{
		::RegCloseKey(hKeyInproc);
		hKeyInproc = nullptr;
	}

	if (hKeyCLSID)
	{
		::RegCloseKey(hKeyCLSID);
		hKeyCLSID = nullptr;
	}

	if (hKeyDriveHandler)
	{
		::RegCloseKey(hKeyDriveHandler);
		hKeyDriveHandler = nullptr;
	}

	return hr;
}

/// </ inheritdoc>
HRESULT CheckDllAndOSBitnessMatch(bool& isMatch)
{
	HRESULT hr = S_OK;
	isMatch = false;

	WCHAR szModulePath[MAX_PATH] = {};

	hr = ::GetModuleFileNameW(szModulePath, MAX_PATH);
	if (FAILED(hr))
	{
		return hr;
	}

	bool dllIs64Bit = false;
	hr = IsDll64Bit(szModulePath, dllIs64Bit);
	if (FAILED(hr))
	{
		return hr;
	}

	bool osIs64Bit = false;
	hr = IsCurrentOS64Bit(osIs64Bit);
	if (FAILED(hr))
	{
		return hr;
	}

	isMatch = (dllIs64Bit == osIs64Bit);
	return S_OK;
}

/// </inheritdoc>
HRESULT GetModuleFileNameW(LPWSTR szModulePath, DWORD dwSize)
{
	// Get the full path of the module
	if (!::GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), szModulePath, dwSize))
	{
		DWORD dwLastError = GetLastError();
		return HRESULT_FROM_WIN32(dwLastError);
	}

	return S_OK;
}

/// </ inheritdoc>
HRESULT IsDll64Bit(const wchar_t* dllPath, bool& is64Bit)
{
	is64Bit = false;
	PIMAGE_NT_HEADERS ntHeaders;

	HANDLE hFile = ::CreateFileW(dllPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HANDLE hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!hMapping)
	{
		::CloseHandle(hFile);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	LPVOID lpBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	if (!lpBase)
	{
		::CloseHandle(hMapping);
		CloseHandle(hFile);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr = S_OK;
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)lpBase;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		hr = E_FAIL;
		goto End;
	}

	ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)lpBase + dosHeader->e_lfanew);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		hr = E_FAIL;
		goto End;
	}

	if (ntHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		is64Bit = true;
	else if (ntHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		is64Bit = false;
	else
		hr = E_FAIL;

End:

	UnmapViewOfFile(lpBase);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return hr;
}

/// </ inheritdoc>
HRESULT IsCurrentOS64Bit(bool& is64Bit)
{
	is64Bit = false;
	SYSTEM_INFO si = {};
	GetNativeSystemInfo(&si);

	switch (si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
	case PROCESSOR_ARCHITECTURE_IA64:
		is64Bit = true;
		break;
	case PROCESSOR_ARCHITECTURE_INTEL:
		is64Bit = false;
		break;
	default:
		return E_FAIL;
	}
	return S_OK;
}

