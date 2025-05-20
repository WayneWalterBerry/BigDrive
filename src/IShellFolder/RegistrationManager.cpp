// <copyright file="RegistrationManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <debugapi.h>
#include <objbase.h>
#include <sstream>
#include <shobjidl.h>

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

/// <summary>
/// Drive Ids are stored in the registry as GUIDs. This function retrieves the GUIDs
/// of all registered drives, which are the same as the CLSIDs of the shell folders.
/// </summary>
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
    hr = StringFromGUID(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
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
    hr = StringFromGUID(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
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

    // Build CLSID path: "CLSID\{guid}\InprocServer32"
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

    // Build CLSID path: "CLSID\{guid}\InprocServer32"
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

    dwAttributes = SFGAO_FOLDER | SFGAO_FILESYSANCESTOR | SFGAO_HASSUBFOLDER | SFGAO_STORAGEANCESTOR | SFGAO_STORAGE | SFGAO_FILESYSTEM;
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
    LSTATUS lResult;

    HKEY hKey = nullptr;
    HKEY hClsidKey = nullptr;

    // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    WCHAR szDriveGuid[39];
    WCHAR namespacePath[256];
    WCHAR componentCategoryPath[] = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

    // Register under CLSID\{guid}\InprocServer32
    hr = RegisterInprocServer32(guidDrive, bstrName);
    if (FAILED(hr))
    {
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to register InprocServer32 for GUID: %s", szDriveGuid);
        goto End;
    }

    // Convert the GUID to a string
    hr = StringFromGUID(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
    if (FAILED(hr))
    {
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to convert GUID to string: %s", szDriveGuid);
        return hr;
    }

    // Register as a Drive (directly as a ShellFolder)
    // Regedit32.exe Path: Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace
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

    hr = RegisterDefaultIcon(guidDrive);
    if (FAILED(hr))
    {
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to register default icon for GUID: %s", szDriveGuid);
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
    std::wstring clsidPath;
    std::wstring namespacePath;
    std::wstring componentCategoryPath = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

    // Convert the GUID to a string
    if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
    {
        WriteError(guid, L"Failed to convert GUID to string");
        return E_FAIL;
    }

    // Delete the CLSID registry key
    clsidPath = L"CLSID\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidPath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", clsidPath.c_str(), dwLastError);
        hr = HRESULT_FROM_WIN32(dwLastError);
    }

    // Delete the namespace registry key
    namespacePath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CURRENT_USER, namespacePath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", namespacePath.c_str(), dwLastError);
        hr = HRESULT_FROM_WIN32(dwLastError);
    }

    // Delete the component category registry key
    componentCategoryPath += L"\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, componentCategoryPath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", componentCategoryPath.c_str(), dwLastError);
        hr = HRESULT_FROM_WIN32(dwLastError);
    }

    return hr;
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