// <copyright file="RegistrationManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// EXPLICIT_ACCESSW is defined in <Aclapi.h>, but it depends on #define 
// UNICODE being set before including Windows headers. In most modern projects,
// this is already set, but you should verify it.
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

// System
#include <windows.h>
#include <debugapi.h>   
#include <objbase.h>
#include <shobjidl.h>
#include <Aclapi.h>

// Header
#include "RegistrationManager.h"

// Local
#include "LaunchDebugger.h"

// BigDrive.Client
#include "..\BigDrive.Client\BigDriveClientConfigurationManager.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;
BigDriveShellFolderEventLogger RegistrationManager::s_eventLogger(L"BigDrive.ShellFolder");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// </inheritdoc>
HRESULT RegistrationManager::RegisterShellFoldersFromRegistry()
{
	HRESULT hr = S_OK;
	GUID* pGuids = nullptr;
	DWORD dwSize = 0;
	DWORD index = 0;

	// Get the drive GUIDs from the registry
	hr = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);
	if (FAILED(hr))
	{
		goto End;
	}

	// Register each drive
	for (DWORD i = 0; i < dwSize; ++i)
	{
		GUID guid = pGuids[index];
		DriveConfiguration driveConfiguration;

		// Get the configuration for the drive from the COM++ BigDrive.Service
		hr = GetConfiguration(guid, driveConfiguration);
		if (FAILED(hr))
		{
			WriteError(guid, L"Failed to get drive configuration for drive");
			break;
		}

		hr = RegisterShellFolder(pGuids[index], driveConfiguration.name);
		if (FAILED(hr))
		{
			WriteError(guid, L"Failed to register shell folder for drive.");
			break;
		}

		WriteInfoFormmated(guid, L"Named: %s Registered As An IShellFolder", driveConfiguration.name);

		++index;
	}

End:

	if (pGuids)
	{
		::CoTaskMemFree(pGuids);
		pGuids = nullptr;
	}

	return hr;
}

/// </inheritdoc>
HRESULT RegistrationManager::GetRegisteredCLSIDs(CLSID** ppClsids, DWORD& dwSize)
{
	HRESULT hr = S_OK;
	GUID* pGuids = nullptr;

	// Get the Drive GUIDs from the registry
	hr = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);
	if (FAILED(hr))
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to get drive GUIDs from registry. HRESULT: 0x%08X", hr);
		goto End;
	}

	// Convert GUIDs to CLSIDs
	*ppClsids = static_cast<CLSID*>(::CoTaskMemAlloc(sizeof(CLSID) * dwSize));
	if (*ppClsids == nullptr)
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to allocate memory for CLSIDs");
		hr = E_OUTOFMEMORY;
		goto End;
	}

	for (DWORD i = 0; i < dwSize; ++i)
	{
		(*ppClsids)[i] = pGuids[i];
	}

End:

	if (pGuids)
	{
		::CoTaskMemFree(pGuids);
		pGuids = nullptr;
	}

	return hr;
}

/// </inheritdoc>
HRESULT RegistrationManager::GetConfiguration(GUID guid, DriveConfiguration& driveConfiguration)
{
	return BigDriveConfigurationClient::GetDriveConfiguration(guid, driveConfiguration);
}

/// </inheritdoc>
HRESULT RegistrationManager::GetModuleFileNameW(LPWSTR szModulePath, DWORD dwSize)
{
	// Get the full path of the module
	if (!::GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), szModulePath, dwSize))
	{
		DWORD dwLastError = GetLastError();
		s_eventLogger.WriteErrorFormmated(L"Failed to get module file name: %s, Error: %u", szModulePath, dwLastError);
		return HRESULT_FROM_WIN32(dwLastError);
	}

	return S_OK;
}

/// </inheritdoc>
HRESULT RegistrationManager::RegisterDefaultIcon(GUID guidDrive)
{
	WCHAR szDriveGuid[39];
	WCHAR defaultIconKeyPath[128];
	HKEY hKey = nullptr;
	HRESULT hr = S_OK;

	// Convert the GUID to a string
	hr = ::StringFromGUID2(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterDefaultIcon: Failed to convert GUID to string: %s", szDriveGuid);
		return hr;
	}

	// Build the registry path: CLSID\{guid}\DefaultIcon
	swprintf_s(defaultIconKeyPath, ARRAYSIZE(defaultIconKeyPath), L"CLSID\\%s\\DefaultIcon", szDriveGuid);

	// Create or open the DefaultIcon key
	LONG lResult = RegCreateKeyExW(
		HKEY_CLASSES_ROOT,
		defaultIconKeyPath,
		0,
		nullptr,
		0,
		KEY_WRITE,
		nullptr,
		&hKey,
		nullptr
	);

	if (lResult != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterDefaultIcon: Failed to create/open registry key: %s, Error: %u", defaultIconKeyPath, dwLastError);
		return HRESULT_FROM_WIN32(dwLastError);
	}

	// Set the default value to the standard drive icon
	const wchar_t* iconValue = L"%SystemRoot%\\System32\\imageres.dll,-30";
	DWORD cbData = static_cast<DWORD>((wcslen(iconValue) + 1) * sizeof(wchar_t));
	lResult = RegSetValueExW(
		hKey,
		nullptr, // Default value
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(iconValue),
		cbData
	);

	if (lResult != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterDefaultIcon: Failed to set DefaultIcon value, Error: %u", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
	}

	if (hKey)
	{
		RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

/// <inheritdoc />
HRESULT RegistrationManager::RegisterInprocServer32(GUID guidDrive, BSTR bstrName)
{
	HRESULT hr = S_OK;
	LRESULT lResult;

	HKEY hKey = nullptr;
	HKEY hShellFolderKey = nullptr;
	const BYTE* lpData = nullptr;
	DWORD cbSize = 0;
	DWORD nameLen;
	DWORD dwAttributes;

	WCHAR szDriveGuid[39];
	WCHAR clsidPath[128];
	WCHAR implementedCategoriesPath[256];
	WCHAR shellFolderPath[128];
	WCHAR szModulePath[MAX_PATH];
	WCHAR szFolderType[128];

	// Convert the GUID to a string
	hr = ::StringFromGUID2(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to convert GUID to string: %s", szDriveGuid);
		return hr;
	}

	// Get the full path of the module
	hr = GetModuleFileNameW(szModulePath, MAX_PATH);
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to get module file name: %s, HRESULT: 0x%08X", szModulePath, hr);
		return hr;
	}

	// Build CLSID path: "CLSID\%s"
	::swprintf_s(clsidPath, ARRAYSIZE(clsidPath), L"CLSID\\%s", szDriveGuid);
	if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidPath, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", clsidPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	// Set a default value (display name for the drive)
	nameLen = SysStringByteLen(bstrName) + sizeof(WCHAR);
	if (::RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(bstrName), nameLen) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", bstrName, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	// Build CLSID path: "CLSID\%s\InprocServer32"
	::swprintf_s(clsidPath, ARRAYSIZE(clsidPath), L"CLSID\\%s\\InprocServer32", szDriveGuid);
	if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidPath, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", clsidPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	cbSize = static_cast<DWORD>((wcslen(szModulePath) + 1) * sizeof(WCHAR));
	lpData = reinterpret_cast<const BYTE*>(szModulePath);
	if (::RegSetValueExW(hKey, nullptr, 0, REG_SZ, lpData, cbSize) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", szModulePath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	if (::RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Apartment"), sizeof(L"Apartment")) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", L"ThreadingModel", dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	// Register in Implemented Categories for Shell Folder (and drive emulation)
	// {00021490-0000-0000-C000-000000000046} is the CATID_ShellFolder
	::swprintf_s(implementedCategoriesPath, ARRAYSIZE(implementedCategoriesPath), L"CLSID\\%s\\Implemented Categories\\{00021490-0000-0000-C000-000000000046}", szDriveGuid);
	if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, implementedCategoriesPath, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", implementedCategoriesPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	// Register ShellFolder attributes
	::swprintf_s(shellFolderPath, ARRAYSIZE(shellFolderPath), L"CLSID\\%s\\ShellFolder", szDriveGuid);
	if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, shellFolderPath, 0, nullptr, 0, KEY_WRITE, nullptr, &hShellFolderKey, nullptr) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", shellFolderPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	dwAttributes = SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_FILESYSANCESTOR;
	lResult = ::RegSetValueExW(hShellFolderKey, L"Attributes", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwAttributes), sizeof(dwAttributes));
	if (lResult != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", shellFolderPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	::swprintf_s(szFolderType, ARRAYSIZE(szFolderType), L"Storage");
	if (::RegSetValueExW(hShellFolderKey, L"FolderType", 0, REG_SZ, reinterpret_cast<const BYTE*>(&szFolderType), sizeof(szFolderType)) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", szFolderType, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

End:

	if (hShellFolderKey)
	{
		::RegCloseKey(hShellFolderKey);
		hShellFolderKey = nullptr;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

/// <inheritdoc />
HRESULT RegistrationManager::RegisterShellFolder(GUID guidDrive, BSTR bstrName)
{
	HRESULT hr = S_OK;
	HKEY hKey = nullptr;
	WCHAR szDriveGuid[39];
	WCHAR namespacePath[256];
	WCHAR componentCategoryPath[] = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

	// Register under CLSID\{guid}\InprocServer32 and related registry entries
	// This sets up the COM registration and ShellFolder attributes
	hr = RegisterInprocServer32(guidDrive, bstrName);
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to register InprocServer32 for GUID: %s", szDriveGuid);
		goto End;
	}

	// Convert the GUID to a string for use in registry paths
	hr = ::StringFromGUID2(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to convert GUID to string: %s", szDriveGuid);
		return hr;
	}

	// Register as a Drive (directly as a ShellFolder) in the user's namespace
	// Path: HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{guid}
	::swprintf_s(namespacePath, ARRAYSIZE(namespacePath), L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\%s", szDriveGuid);
	if (::RegCreateKeyExW(HKEY_CURRENT_USER, namespacePath, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", namespacePath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

	if (hKey)
	{
		RegCloseKey(hKey);
		hKey = nullptr;
	}

	// Take ownership and grant full control of the Component Category registry key,
	// then create the {guid} subkey under Implementations
	hr = TakeOwnershipAndGrantFullControl(&CreateComponentCategoryRegistryKey, guidDrive);
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to take ownership and grant full control for registry key: %s", componentCategoryPath);
		goto End;
	}

	// Register the DefaultIcon for the ShellFolder
	hr = RegisterDefaultIcon(guidDrive);
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to register default icon for GUID: %s", szDriveGuid);
		goto End;
	}

End:

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

/// <inheritdoc />
HRESULT RegistrationManager::CreateComponentCategoryRegistryKey(GUID guidDrive)
{
	HRESULT hr = S_OK;
	LRESULT lResult = 0;
	WCHAR szDriveGuid[39];
	HKEY hKey = nullptr;
	HKEY hClsidKey = nullptr;
	WCHAR componentCategoryPath[] = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

	// Convert the GUID to a string
	// GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

	hr = ::StringFromGUID2(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
	if (FAILED(hr))
	{
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to convert GUID to string: %s", szDriveGuid);
		return hr;
	}

	// Register in Component Categories
	lResult = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, componentCategoryPath, 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		// Try to create if not found
		lResult = ::RegCreateKeyExW(HKEY_CLASSES_ROOT, componentCategoryPath, 0, nullptr, 0, KEY_WRITE | KEY_WOW64_64KEY, nullptr, &hKey, nullptr);
		if (lResult != ERROR_SUCCESS)
		{
			DWORD dwLastError = GetLastError();
			WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create/open registry key: %s, Error: %u", componentCategoryPath, dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto End;
		}
	}

	// Add your CLSID as a subkey under "Implementations"
	lResult = ::RegCreateKeyExW(hKey, szDriveGuid, 0, nullptr, 0, KEY_WRITE, nullptr, &hClsidKey, nullptr);
	if (lResult != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", szDriveGuid, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

End:

	if (hClsidKey)
	{
		::RegCloseKey(hClsidKey);
		hClsidKey = nullptr;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

/// </ inheritdoc>
HRESULT RegistrationManager::UnregisterShellFolder(GUID guid)
{
	HRESULT hr = S_OK;
	wchar_t guidString[39];
	WCHAR clsidPath[128];
	WCHAR namespacePath[256];
	WCHAR componentCategoryPath[256] = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

	// Convert the GUID to a string
	if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
	{
		WriteError(guid, L"Failed to convert GUID to string");
		return E_FAIL;
	}

	// Delete the CLSID registry key
	swprintf_s(clsidPath, ARRAYSIZE(clsidPath), L"CLSID\\%s", guidString);
	if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidPath) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", clsidPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
	}

	// Delete the namespace registry key
	swprintf_s(namespacePath, ARRAYSIZE(namespacePath), L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\%s", guidString);
	if (::RegDeleteTreeW(HKEY_CURRENT_USER, namespacePath) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", namespacePath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
	}

	hr = RegistrationManager::TakeOwnershipAndGrantFullControl(&DeleteComponentCategoryRegistryKey, guid);
	if (FAILED(hr))
	{
		WriteErrorFormmated(guid, L"Failed to take ownership and grant full control for registry key: %s", componentCategoryPath);
		goto End;
	}

End:

	return hr;
}

HRESULT RegistrationManager::DeleteComponentCategoryRegistryKey(GUID guid)
{
	HRESULT hr = S_OK;
	WCHAR guidString[39];
	WCHAR componentCategoryPath[256] = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

	// Convert the GUID to a string
	if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
	{
		WriteError(guid, L"Failed to convert GUID to string");
		return E_FAIL;
	}

	// Delete the component category registry key
	swprintf_s(componentCategoryPath + wcslen(componentCategoryPath), ARRAYSIZE(componentCategoryPath) - wcslen(componentCategoryPath), L"\\%s", guidString);
	if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, componentCategoryPath) != ERROR_SUCCESS)
	{
		DWORD dwLastError = GetLastError();
		WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", componentCategoryPath, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto End;
	}

End:

	return S_OK;
}

/// </ inheritdoc>
HRESULT RegistrationManager::CleanUpShellFolders()
{
	HRESULT hr = S_OK;

	HKEY hKey = nullptr;
	DWORD index = 0;
	WCHAR szClsid[256];
	DWORD subKeyNameSize = ARRAYSIZE(szClsid);

	// Open the CLSID registry key
	LONG result = RegOpenKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS)
	{
		return HRESULT_FROM_WIN32(result);
	}

	// Enumerate all subkeys under CLSID
	while (::RegEnumKeyEx(hKey, index, szClsid, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
	{
		HKEY hSubKey = nullptr;
		WCHAR inprocServerPath[512];
		DWORD valueSize = sizeof(inprocServerPath);

		// Construct the path to the InprocServer32 subkey
		WCHAR inprocServerKeyPath[512];
		swprintf_s(inprocServerKeyPath, ARRAYSIZE(inprocServerKeyPath), L"CLSID\\%s\\InprocServer32", szClsid);

		// Open the InprocServer32 subkey
		result = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, inprocServerKeyPath, 0, KEY_READ, &hSubKey);
		if (result == ERROR_SUCCESS)
		{
			// Query the default value of the InprocServer32 subkey
			result = ::RegQueryValueEx(hSubKey, nullptr, nullptr, nullptr, reinterpret_cast<LPBYTE>(inprocServerPath), &valueSize);
			if (result == ERROR_SUCCESS)
			{
				// Close the subkey
				if (hSubKey)
				{
					::RegCloseKey(hSubKey);
					hSubKey = nullptr;
				}

				// Check if the value contains "BigDrive.ShellFolder"
				if (wcsstr(inprocServerPath, L"BigDrive.ShellFolder") != nullptr)
				{
					GUID guid = GUID_NULL;

					hr = GUIDFromString(szClsid, &guid);
					if (FAILED(hr))
					{
						WriteErrorFormmated(guid, L"CleanUpShellFolders() failed to convert CLSID to GUID");
						break;
					}

					hr = UnregisterShellFolder(guid);
					if (FAILED(hr))
					{
						WriteErrorFormmated(guid, L"UnregisterShellFolder() failed.");
						break;
					}
				}
			}

			// Close the subkey
			if (hSubKey)
			{
				::RegCloseKey(hSubKey);
				hSubKey = nullptr;
			}
		}

		// Reset the subKeyNameSize and increment the index
		subKeyNameSize = ARRAYSIZE(szClsid);
		++index;
	}

	// Close the CLSID key
	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

/// <summary>
/// Logs a formatted info message with the Drive Guid
/// </summary>
/// <param name="formatter">The format string for the error message.</param>
/// <param name="...">The arguments for the format string.</param>
/// <returns>HRESULT indicating success or failure of the logging operation.</returns>
HRESULT RegistrationManager::WriteInfoFormmated(GUID guid, LPCWSTR formatter, ...)
{
	va_list args;

	va_start(args, formatter);
	wchar_t buffer[1024];
	::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);
	va_end(args);

	// Format the GUID components
	wchar_t guidBuffer[128];
	swprintf(
		guidBuffer,
		sizeof(guidBuffer) / sizeof(wchar_t),
		L"[RegistrationManager] - Drive: {%08lX-%04X-%04X-%02X%02X-%02X%02X-%02X%02X-%02X%02X} ",
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
	);

	// Prepend the GUID to the error message
	wchar_t finalBuffer[1024];
	swprintf(
		finalBuffer,
		sizeof(finalBuffer) / sizeof(wchar_t),
		L"%s%s",
		guidBuffer,
		buffer
	);

	return s_eventLogger.WriteInfo(finalBuffer);
}

/// <summary>
/// Logs an error message with the CLSID of the provider.
/// </summary>
/// <param name="message">The error message to log.</param>
/// <returns>HRESULT indicating success or failure of the logging operation.</returns>
HRESULT RegistrationManager::WriteError(GUID guid, LPCWSTR message)
{
	return s_eventLogger.WriteErrorFormmated(
		L"[RegistrationManager] - Provider: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X} %s",
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7],
		message);
}

/// <summary>
/// Logs a formatted error message with the Drive Guid
/// </summary>
/// <param name="formatter">The format string for the error message.</param>
/// <param name="...">The arguments for the format string.</param>
/// <returns>HRESULT indicating success or failure of the logging operation.</returns>
HRESULT RegistrationManager::WriteErrorFormmated(GUID guid, LPCWSTR formatter, ...)
{
	va_list args;

	va_start(args, formatter);
	wchar_t buffer[1024];
	::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);
	va_end(args);

	// Format the GUID components
	wchar_t guidBuffer[128];
	swprintf(
		guidBuffer,
		sizeof(guidBuffer) / sizeof(wchar_t),
		L"[RegistrationManager] - Drive: {%08lX-%04X-%04X-%02X%02X-%02X%02X-%02X%02X-%02X%02X} ",
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
	);

	// Prepend the GUID to the error message
	wchar_t finalBuffer[1024];
	swprintf(
		finalBuffer,
		sizeof(finalBuffer) / sizeof(wchar_t),
		L"%s%s",
		guidBuffer,
		buffer
	);

	return s_eventLogger.WriteError(finalBuffer);
}

/// </ inheritdoc>
HRESULT RegistrationManager::IsDll64Bit(const wchar_t* dllPath, bool& is64Bit)
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
HRESULT RegistrationManager::IsCurrentOS64Bit(bool& is64Bit)
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

/// </ inheritdoc>
HRESULT RegistrationManager::CheckDllAndOSBitnessMatch(bool& isMatch)
{
	HRESULT hr = S_OK;
	isMatch = false;

	WCHAR szModulePath[MAX_PATH] = {};

	hr = GetModuleFileNameW(szModulePath, MAX_PATH);
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

/// </ inheritdoc>
HRESULT RegistrationManager::TakeOwnershipAndGrantFullControl(HRESULT(*callback)(GUID), GUID guid)
{
	HRESULT hr = S_OK;
	LPCWSTR keyPath = L"Component Categories\\{00021493-0000-0000-C000-000000000046}";
	HKEY hKey = nullptr;
	LONG lRes = 0;
	PSECURITY_DESCRIPTOR pOrigOwnerSD = nullptr;
	DWORD origOwnerSDSize = 0;
	PSECURITY_DESCRIPTOR pOrigDaclSD = nullptr;
	DWORD origDaclSDSize = 0;
	DWORD sdSize = 0;
	PSECURITY_DESCRIPTOR pSD = nullptr;
	BOOL bOwnerDefaulted = FALSE;
	BOOL isOwner = FALSE;
	SECURITY_DESCRIPTOR sd;
	EXPLICIT_ACCESSW ea = {};
	PACL pOldDACL = nullptr;
	PACL pNewDACL = nullptr;
	DWORD dwRes = 0;
	SECURITY_DESCRIPTOR sdNew;
	PSID psidCurrentUser = nullptr;
	BOOL ownerDefaulted = FALSE;
	PSID pOwner = nullptr;

	if (!callback)
	{
		hr = E_POINTER;
		WriteError(guid, L"TakeOwnershipAndGrantFullControl: callback pointer is null.");
		goto End;
	}

	// 1. Open with READ_CONTROL to read OWNER and DACL
	lRes = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, READ_CONTROL, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to open registry key '%s' for READ_CONTROL. Error: %u", keyPath, lRes);
		goto End;
	}

	// Save original OWNER
	lRes = ::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, nullptr, &origOwnerSDSize);
	if (lRes != ERROR_INSUFFICIENT_BUFFER)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get size for OWNER_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	pOrigOwnerSD = (PSECURITY_DESCRIPTOR)::malloc(origOwnerSDSize);
	if (!pOrigOwnerSD)
	{
		hr = E_OUTOFMEMORY;
		WriteError(guid, L"TakeOwnershipAndGrantFullControl: Failed to allocate memory for original owner security descriptor.");
		goto End;
	}

	lRes = ::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, pOrigOwnerSD, &origOwnerSDSize);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get OWNER_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	// Save original DACL
	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, nullptr, &origDaclSDSize);
	if (lRes != ERROR_INSUFFICIENT_BUFFER)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get size for DACL_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	pOrigDaclSD = (PSECURITY_DESCRIPTOR)::malloc(origDaclSDSize);
	if (!pOrigDaclSD)
	{
		hr = E_OUTOFMEMORY;
		WriteError(guid, L"TakeOwnershipAndGrantFullControl: Failed to allocate memory for original DACL security descriptor.");
		goto End;
	}

	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pOrigDaclSD, &origDaclSDSize);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get DACL_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	::RegCloseKey(hKey);
	hKey = nullptr;

	// 2. Take ownership if needed (open with WRITE_OWNER)
	hr = GetCurrentProcessSID(&psidCurrentUser);
	if (FAILED(hr))
	{
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get current process SID. HRESULT: 0x%08X", hr);
		goto End;
	}

	// Check if current user is already owner
	if (::GetSecurityDescriptorOwner(pOrigOwnerSD, &pOwner, &ownerDefaulted) && pOwner && psidCurrentUser && ::EqualSid(pOwner, psidCurrentUser))
	{
		isOwner = TRUE;
	}

	if (!isOwner)
	{
		hr = EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
		if (FAILED(hr))
		{
			WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to enable SeTakeOwnershipPrivilege. HRESULT: 0x%08X", hr);
			goto End;
		}

		lRes = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, WRITE_OWNER | READ_CONTROL, &hKey);
		if (lRes != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lRes);
			WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to open registry key '%s' for WRITE_OWNER. Error: %u", keyPath, lRes);
			goto End;
		}

		if (!::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		{
			hr = HRESULT_FROM_WIN32(::GetLastError());
			WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to initialize security descriptor. Error: %u", ::GetLastError());
			goto End;
		}

		if (!::SetSecurityDescriptorOwner(&sd, psidCurrentUser, FALSE))
		{
			hr = HRESULT_FROM_WIN32(::GetLastError());
			WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to set security descriptor owner. Error: %u", ::GetLastError());
			goto End;
		}

		lRes = ::RegSetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, &sd);
		if (lRes != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lRes);
			WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to set key owner. Error: %u", lRes);
			goto End;
		}

		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	// 3. Grant full control (open with WRITE_DAC)
	lRes = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, WRITE_DAC | READ_CONTROL, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to open registry key '%s' for WRITE_DAC. Error: %u", keyPath, lRes);
		goto End;
	}

	ea.grfAccessPermissions = KEY_ALL_ACCESS;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea.Trustee.ptstrName = (LPWSTR)psidCurrentUser;

	// Get current DACL
	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, nullptr, &sdSize);
	if (lRes != ERROR_INSUFFICIENT_BUFFER)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get size for DACL_SECURITY_INFORMATION (WRITE_DAC). Error: %u", lRes);
		goto End;
	}

	pSD = (PSECURITY_DESCRIPTOR)::malloc(sdSize);
	if (!pSD)
	{
		hr = E_OUTOFMEMORY;
		WriteError(guid, L"TakeOwnershipAndGrantFullControl: Failed to allocate memory for DACL security descriptor.");
		goto End;
	}

	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pSD, &sdSize);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get DACL_SECURITY_INFORMATION (WRITE_DAC). Error: %u", lRes);
		goto End;
	}

	if (!::GetSecurityDescriptorDacl(pSD, &bOwnerDefaulted, &pOldDACL, &bOwnerDefaulted))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to get security descriptor DACL. Error: %u", ::GetLastError());
		goto End;
	}

	dwRes = ::SetEntriesInAclW(1, &ea, pOldDACL, &pNewDACL);
	if (dwRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(dwRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to set entries in ACL. Error: %u", dwRes);
		goto End;
	}

	if (!::InitializeSecurityDescriptor(&sdNew, SECURITY_DESCRIPTOR_REVISION))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to initialize new security descriptor. Error: %u", ::GetLastError());
		goto End;
	}

	if (!::SetSecurityDescriptorDacl(&sdNew, TRUE, pNewDACL, FALSE))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to set new security descriptor DACL. Error: %u", ::GetLastError());
		goto End;
	}

	lRes = ::RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION, &sdNew);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		WriteErrorFormmated(guid, L"TakeOwnershipAndGrantFullControl: Failed to set key DACL. Error: %u", lRes);
		goto End;
	}

	// 4. Call the callback
	hr = callback(guid);

	// 5. Restore original OWNER and DACL
	::RegSetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, pOrigOwnerSD);
	::RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pOrigDaclSD);

End:
	if (pNewDACL)
	{
		::LocalFree(pNewDACL);
		pNewDACL = nullptr;
	}

	if (pSD)
	{
		::free(pSD);
		pSD = nullptr;
	}

	if (psidCurrentUser)
	{
		::free(psidCurrentUser);
		psidCurrentUser = nullptr;
	}

	if (pOrigOwnerSD)
	{
		::free(pOrigOwnerSD);
		pOrigOwnerSD = nullptr;
	}

	if (pOrigDaclSD)
	{
		::free(pOrigDaclSD);
		pOrigDaclSD = nullptr;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

/// </ inheritdoc>
HRESULT RegistrationManager::GetCurrentProcessSID(PSID* pOwner)
{
	HRESULT hr = S_OK;
	HANDLE hToken = nullptr;
	DWORD dwLen = 0;
	PTOKEN_USER pTokenUser = nullptr;
	DWORD sidLen = 0;

	if (!pOwner)
	{
		hr = E_POINTER;
		WriteErrorFormmated(GUID_NULL, L"GetCurrentProcessSID: pOwner pointer is null.");
		goto End;
	}

	*pOwner = nullptr;

	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"GetCurrentProcessSID: OpenProcessToken failed. Error: %u", ::GetLastError());
		goto End;
	}

	// First call to get required buffer size
	::GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLen);
	pTokenUser = (PTOKEN_USER)::malloc(dwLen);
	if (!pTokenUser)
	{
		hr = E_OUTOFMEMORY;
		WriteError(GUID_NULL, L"GetCurrentProcessSID: Failed to allocate memory for TOKEN_USER.");
		goto End;
	}

	if (!::GetTokenInformation(hToken, TokenUser, pTokenUser, dwLen, &dwLen))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"GetCurrentProcessSID: GetTokenInformation failed. Error: %u", ::GetLastError());
		goto End;
	}

	// Duplicate the SID for the caller
	sidLen = ::GetLengthSid(pTokenUser->User.Sid);
	*pOwner = (PSID)::malloc(sidLen);
	if (!*pOwner)
	{
		hr = E_OUTOFMEMORY;
		WriteError(GUID_NULL, L"GetCurrentProcessSID: Failed to allocate memory for SID.");
		goto End;
	}

	if (!::CopySid(sidLen, *pOwner, pTokenUser->User.Sid))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"GetCurrentProcessSID: CopySid failed. Error: %u", ::GetLastError());
		::free(*pOwner);
		*pOwner = nullptr;
		goto End;
	}

End:

	if (pTokenUser)
	{
		::free(pTokenUser);
		pTokenUser = nullptr;
	}

	if (hToken)
	{
		::CloseHandle(hToken);
		hToken = nullptr;
	}

	return hr;
}

/// </ inheritdoc>
HRESULT RegistrationManager::EnablePrivilege(LPCWSTR privilege)
{
	HRESULT hr = S_OK;
	HANDLE hToken = nullptr;
	TOKEN_PRIVILEGES tp = {};
	LUID luid = {};

	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"EnablePrivilege: OpenProcessToken failed. Error: %u", ::GetLastError());
		goto End;
	}

	if (!::LookupPrivilegeValueW(nullptr, privilege, &luid))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"EnablePrivilege: LookupPrivilegeValueW failed. Error: %u", ::GetLastError());
		goto End;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"EnablePrivilege: AdjustTokenPrivileges failed. Error: %u", ::GetLastError());
		goto End;
	}

	// AdjustTokenPrivileges may succeed but not enable the privilege
	if (::GetLastError() != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		WriteErrorFormmated(GUID_NULL, L"EnablePrivilege: AdjustTokenPrivileges did not enable privilege. Error: %u", ::GetLastError());
		goto End;
	}

End:

	if (hToken)
	{
		::CloseHandle(hToken);
		hToken = nullptr;
	}

	return hr;
}