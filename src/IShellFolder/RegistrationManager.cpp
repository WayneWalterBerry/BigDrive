// <copyright file="RegistrationManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <debugapi.h>
#include <objbase.h>
#include <sstream>
#include <windows.h>

// Header
#include "RegistrationManager.h"

// Local
#include "LaunchDebugger.h"

// BigDrive.Client
#include "..\BigDrive.Client\BigDriveClientConfigurationManager.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"

// Shared
#include "..\Shared\EventLogger.h"

extern "C" IMAGE_DOS_HEADER __ImageBase; // Correct declaration of __ImageBase

EventLogger RegistrationManager::s_eventLogger(L"BigDrive.ShellFolder");

HRESULT RegistrationManager::RegisterShellFoldersFromRegistry()
{
    HRESULT hrReturn = S_OK;
    GUID* pGuids = nullptr;
    DWORD dwSize = 0;
    DWORD index = 0;

    // Get the drive GUIDs from the registry
    hrReturn = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);
    if (FAILED(hrReturn))
    {
        goto End;
    }

    // Register each drive
    for (DWORD i = 0; i < dwSize; ++i)
    {
        GUID guid = pGuids[index];
        DriveConfiguration driveConfiguration;

        // Get the configuration for the drive from the COM++ BigDrive.Service
        hrReturn = GetConfiguration(guid, driveConfiguration);
        if (FAILED(hrReturn))
        {
            WriteError(guid, L"Failed to get drive configuration for drive");
            break;
        }

        hrReturn = RegisterShellFolder(pGuids[index], driveConfiguration.name);
        if (FAILED(hrReturn))
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

    return hrReturn;
}

/// <summary>
/// Drive Ids are stored in the registry as GUIDs. This function retrieves the GUIDs
/// of all registered drives, which are the same as the CLSIDs of the shell folders.
/// </summary>
HRESULT RegistrationManager::GetRegisteredCLSIDs(CLSID** ppClsids, DWORD& dwSize)
{
    HRESULT hrReturn = S_OK;
    GUID* pGuids = nullptr;

    // Get the Drive GUIDs from the registry
    hrReturn = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to get drive GUIDs from registry. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Convert GUIDs to CLSIDs
    *ppClsids = static_cast<CLSID*>(::CoTaskMemAlloc(sizeof(CLSID) * dwSize));
    if (*ppClsids == nullptr)
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to allocate memory for CLSIDs");
        hrReturn = E_OUTOFMEMORY;
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

    return hrReturn;
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

/// </ inheritdoc>
HRESULT RegistrationManager::RegisterShellFolder(GUID guidDrive, BSTR bstrName)
{
    HRESULT hrReturn = S_OK;
    LSTATUS lResult;

    wchar_t szModulePath[MAX_PATH];
    HKEY hKey = nullptr;
    HKEY hClsidKey = nullptr;
    const BYTE* lpData = nullptr;
    DWORD cbSize = 0;

    // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    wchar_t guidString[39];

    std::wstring clsidPath;
    std::wstring namespacePath;
    std::wstring componentCategoryPath = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

    // Convert the GUID to a string
    hrReturn = StringFromGUID(guidDrive, guidString, ARRAYSIZE(guidString));
    if (FAILED(hrReturn))
    {
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to convert GUID to string: %s", guidString);
        return hrReturn;
    }

    // Get the full path of the module
    hrReturn = GetModuleFileNameW(szModulePath, MAX_PATH);
    if (FAILED(hrReturn))
    {
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to get module file name: %s, HRESULT: 0x%08X", szModulePath, hrReturn);
        return hrReturn;
    }

    clsidPath = L"CLSID\\" + std::wstring(guidString) + L"\\InprocServer32";
    if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", clsidPath.c_str(), dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    cbSize = static_cast<DWORD>((wcslen(szModulePath) + 1) * sizeof(wchar_t));
    lpData = reinterpret_cast<const BYTE*>(szModulePath);
    lResult = ::RegSetValueExW(hKey, nullptr, 0, REG_SZ, lpData, cbSize);
    if (lResult != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", szModulePath, dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    if (::RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Apartment"), sizeof(L"Apartment")) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", L"Apartment", dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    if (hKey)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    // Register as a Drive (directly as a ShellFolder) - CHANGED TO USE HKEY_CURRENT_USER
    namespacePath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\" + std::wstring(guidString);
    if (::RegCreateKeyExW(HKEY_CURRENT_USER, namespacePath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", namespacePath.c_str(), dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    // Set a default value (display name for the drive)
    if (::RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(bstrName), sizeof(bstrName)) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to set registry value: %s, Error: %u", bstrName, dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = nullptr;
    }

    lResult = ::RegOpenKeyExW(HKEY_CLASSES_ROOT,
        L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations",
        0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);
    switch (lResult)
    {
    case ERROR_SUCCESS:
        break;
    case ERROR_FILE_NOT_FOUND:

        lResult = ::RegCreateKeyExW(HKEY_CLASSES_ROOT,
            L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations",
            0, nullptr, 0, KEY_WRITE | KEY_WOW64_64KEY, nullptr, &hKey, nullptr);
        switch (lResult)
        {
        case ERROR_SUCCESS:
            break;
        case ERROR_ACCESS_DENIED:
            /// The TrustedInstaller is a built-in Windows security principal responsible 
            /// for managing system files and updates. It is part of the Windows Modules 
            /// Installer service and controls permissions for core operating system files.
            /// 
            /// Key Functions:
            /// - Protects critical system files from unauthorized modifications.
            /// - Manages Windows Update installations.
            /// - Ensures system stability by restricting access to key components.
            ///
            /// Why It Restricts Access:
            /// - Many system files are "owned" by TrustedInstaller.
            /// - Even administrators may need to take ownership to modify certain files.
            /// - Altering TrustedInstaller-protected files can lead to system instability.
            ///
            /// Changing Ownership (If Necessary):
            /// - Right-click the file → Properties → Security tab → Advanced.
            /// - Change Owner to an administrator or your user account.
            /// - Grant Full Control permissions before modifying.
            ///
            /// Modifying files controlled by TrustedInstaller should be done cautiously, 
            /// as improper changes can break Windows functionality.
            WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Access Denied -- Failed to create registry key: %s, Error: %u",
                L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations", lResult);
            hrReturn = E_FAIL;
            goto End;
        default:
            DWORD dwLastError = GetLastError();
            WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u",
                L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations", dwLastError);
            hrReturn = E_FAIL;
            goto End;
        }

        break;

    default:
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to open registry key: %s, Error: %u",
            L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations", dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    // Add your CLSID as a subkey under "Implementations"
    if (::RegCreateKeyExW(hKey, guidString, 0, nullptr, 0, KEY_WRITE, nullptr, &hClsidKey, nullptr) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guidDrive, L"RegisterShellFolder: Failed to create registry key: %s, Error: %u", guidString, dwLastError);
        hrReturn = E_FAIL;
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

    return hrReturn;
}

/// </ inheritdoc>
HRESULT RegistrationManager::UnregisterShellFolder(GUID guid)
{
    HRESULT hrReturn = S_OK;
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
        hrReturn = HRESULT_FROM_WIN32(dwLastError);
    }

    // Delete the namespace registry key
    namespacePath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CURRENT_USER, namespacePath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", namespacePath.c_str(), dwLastError);
        hrReturn = HRESULT_FROM_WIN32(dwLastError);
    }

    // Delete the component category registry key
    componentCategoryPath += L"\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, componentCategoryPath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteErrorFormmated(guid, L"Failed to delete registry key: %s, Error: %u", componentCategoryPath.c_str(), dwLastError);
        hrReturn = HRESULT_FROM_WIN32(dwLastError);
    }

    return hrReturn;
}

/// <summary>
/// Iterate All Shell Folders and remove the BigDrive ones.
/// </summary>
/// <returns></returns>
HRESULT RegistrationManager::UnregisterShellFolders()
{
    HRESULT hrReturn = S_OK;

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

                    hrReturn = GUIDFromString(szClsid, &guid);
                    if (FAILED(hrReturn))
                    {
                        WriteErrorFormmated(guid, L"CleanUpShellFolders() failed to convert CLSID to GUID");
                        break;
                    }

                    hrReturn = UnregisterShellFolder(guid);
                    if (FAILED(hrReturn))
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

    return hrReturn;
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



