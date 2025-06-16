// <copyright file="dllmain.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "dllmain.h"
#include "LaunchDebugger.h"
#include "BigDriveExtensionClassFactory.h"

#include <windows.h>

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

	hr = CheckDllAndOSBitnessMatch(bitMatch);
	if (FAILED(hr))
	{
		// Log the error and return failure.
		goto End;
	}

	if (!bitMatch)
	{
		// Log a message indicating that the bitness of the DLL and OS do not match.
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
		::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlMyComputer, NULL);
		::CoTaskMemFree(pidlMyComputer);
	}

End:

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
extern "C" HRESULT __stdcall DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	HRESULT hr = S_OK;
	BigDriveExtensionClassFactory* pFactory = nullptr;

	LaunchDebugger(); // Optional: Launch debugger if needed for troubleshooting

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

/// </ inheritdoc>
HRESULT RegisterContextMenuExtension(const CLSID& clsidExtension)
{
	HRESULT hr = S_OK;
	HKEY hKey = nullptr;
	HKEY hKeyInproc = nullptr;
	WCHAR szCLSID[64] = { 0 };
	WCHAR szModulePath[MAX_PATH] = { 0 };
	LONG lResult = 0;

	// Correct registry key for Drive context menu handlers
	LPCWSTR szDriveHandlers = L"Drive\\shellex\\ContextMenuHandlers\\BigDriveExtension";

	// Convert CLSID to string
	if (FAILED(::StringFromGUID2(clsidExtension, szCLSID, ARRAYSIZE(szCLSID))))
	{
		hr = E_FAIL;
		goto End;
	}

	// 1. Register the COM object under HKCR\CLSID\{CLSID_BigDriveExtension}
	lResult = ::RegCreateKeyExW(
		HKEY_CLASSES_ROOT,
		szCLSID,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		nullptr,
		&hKey,
		nullptr);

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

	// Set the default value (optional, but recommended)
	lResult = ::RegSetValueExW(
		hKey,
		nullptr,
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(L"BigDrive Shell Context Menu Extension"),
		static_cast<DWORD>((::lstrlenW(L"BigDrive Shell Context Menu Extension") + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

	// 2. Register InprocServer32 subkey
	lResult = ::RegCreateKeyExW(
		hKey,
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
		goto End;
	}

	// Get the full path to this DLL
	if (!::GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), szModulePath, ARRAYSIZE(szModulePath)))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
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
		goto End;
	}

	// 3. Register the handler under Drive ContextMenuHandlers
	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	lResult = ::RegCreateKeyExW(
		HKEY_CLASSES_ROOT,
		szDriveHandlers,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		nullptr,
		&hKey,
		nullptr);

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

	// Set the default value to the CLSID string
	lResult = ::RegSetValueExW(
		hKey,
		nullptr,
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(szCLSID),
		static_cast<DWORD>((::lstrlenW(szCLSID) + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

End:

	if (hKeyInproc)
	{
		::RegCloseKey(hKeyInproc);
		hKeyInproc = nullptr;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
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

