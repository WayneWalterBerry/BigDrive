// <copyright file="RegistrationManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <debugapi.h>
#include <objbase.h>
#include <sstream>
#include <windows.h>

#include "RegistrationManager.h"
#include "..\BigDrive.Client\BigDriveClientConfigurationProvider.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"

extern "C" IMAGE_DOS_HEADER __ImageBase; // Correct declaration of __ImageBase

/// <summary>
/// Static method to get the singleton instance
/// </summary>
RegistrationManager& RegistrationManager::GetInstance()
{
    static RegistrationManager instance; // Guaranteed to be initialized only once
    return instance;
}

HRESULT RegistrationManager::RegisterShellFoldersFromRegistry()
{
    HRESULT hrReturn = S_OK;
    GUID* pGuids = nullptr;
    DWORD index = 0;

    // Get the drive GUIDs from the registry
    hrReturn = BigDriveClientConfigurationProvider::GetDriveGuids(&pGuids);
    if (FAILED(hrReturn))
    {
        goto End;
    }

    // Register each drive
    while (pGuids[index] != GUID_NULL)
    {
        GUID guid = pGuids[index];
        DriveConfiguration driveConfiguration;

        // Get the configuration for the drive from the COM++ BigDrive.Service
        hrReturn = GetConfiguration(guid, driveConfiguration);
        if (FAILED(hrReturn))
        {
            WriteError(L"Failed to get drive configuration for drive: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                guid.Data1,
                guid.Data2,
                guid.Data3,
                guid.Data4[0], guid.Data4[1],
                guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
            break;
        }

        hrReturn = RegisterShellFolder(pGuids[index], driveConfiguration.name);
        if (FAILED(hrReturn))
        {
            WriteError(
                L"Failed to get drive configuration for drive: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                guid.Data1,
                guid.Data2,
                guid.Data3,
                guid.Data4[0], guid.Data4[1],
                guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
            break;
        }

        WriteInfo(
            L"Drive[{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}] Named: %s Regsitered As An IShellFolder",
            driveConfiguration.name);

        ++index;
    }

End:

    if (pGuids)
    {
        ::CoTaskMemFree(pGuids);
    }

    return hrReturn;
}

/// </inheritdoc>
HRESULT RegistrationManager::GetConfiguration(GUID guid, DriveConfiguration& driveConfiguration)
{
    return BigDriveConfigurationClient::GetDriveConfiguration(guid, driveConfiguration);
}

/// </ inheritdoc>
HRESULT RegistrationManager::RegisterShellFolder(GUID guid, BSTR bstrName)
{
    HRESULT hrReturn = S_OK;
    wchar_t modulePath[MAX_PATH];
    HKEY hKey = nullptr;
    HKEY hClsidKey = nullptr;

    // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    wchar_t guidString[39];

    std::wstring clsidPath;
    std::wstring namespacePath;
    std::wstring componentCategoryPath = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations";

    // Convert the GUID to a string
    if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
    {
        WriteError(
            L"Failed to convert GUID to string: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0], guid.Data4[1],
            guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return E_FAIL;
    }

    // Get the full path of the module
    if (!GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), modulePath, MAX_PATH))
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to get module file name: %s, Error: %u", modulePath, dwLastError);
        return HRESULT_FROM_WIN32(dwLastError);
    }

    // Register CLSID_BigDriveShellFolder

    clsidPath = L"CLSID\\" + std::wstring(guidString) + L"\\InprocServer32";
    if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to create registry key: %s, Error: %u", clsidPath.c_str(), dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    if (::RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(modulePath), static_cast<DWORD>((wcslen(modulePath) + 1) * sizeof(wchar_t)) != ERROR_SUCCESS))
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to set registry value: %s, Error: %u", modulePath, dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    if (::RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Apartment"), sizeof(L"Apartment")) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to set registry value: %s, Error: %u", L"Apartment", dwLastError);
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
        WriteError(L"Failed to create registry key: %s, Error: %u", namespacePath.c_str(), dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    // Set a default value (display name for the drive)
    if (::RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(bstrName), sizeof(bstrName)) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to set registry value: %s, Error: %u", bstrName, dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = nullptr;
    }

    if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, componentCategoryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to create registry key: %s, Error: %u", componentCategoryPath.c_str(), dwLastError);
        hrReturn = E_FAIL;
        goto End;
    }

    // Add your CLSID as a subkey under "Implementations"
    if (::RegCreateKeyExW(hKey, guidString, 0, nullptr, 0, KEY_WRITE, nullptr, &hClsidKey, nullptr) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to create registry key: %s, Error: %u", guidString, dwLastError);
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
        WriteError(
            L"Failed to convert GUID to string: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0], guid.Data4[1],
            guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return E_FAIL;
    }

    // Delete the CLSID registry key
    clsidPath = L"CLSID\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidPath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to delete registry key: %s, Error: %u", clsidPath.c_str(), dwLastError);
        hrReturn = HRESULT_FROM_WIN32(dwLastError);
    }

    // Delete the namespace registry key
    namespacePath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CURRENT_USER, namespacePath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to delete registry key: %s, Error: %u", namespacePath.c_str(), dwLastError);
        hrReturn = HRESULT_FROM_WIN32(dwLastError);
    }

    // Delete the component category registry key
    componentCategoryPath += L"\\" + std::wstring(guidString);
    if (::RegDeleteTreeW(HKEY_CLASSES_ROOT, componentCategoryPath.c_str()) != ERROR_SUCCESS)
    {
        DWORD dwLastError = GetLastError();
        WriteError(L"Failed to delete registry key: %s, Error: %u", componentCategoryPath.c_str(), dwLastError);
        hrReturn = HRESULT_FROM_WIN32(dwLastError);
    }

    return hrReturn;
}


/// </ inheritdoc>
HRESULT RegistrationManager::WriteError(LPCWSTR formatter, ...)
{
    va_list args;
    va_start(args, formatter);
    wchar_t buffer[1024];
    ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);
    va_end(args);

    return EventLogger::GetInstance().WriteError(buffer, EVENTLOG_ERROR_TYPE);
}

/// </ inheritdoc>
HRESULT RegistrationManager::WriteInfo(LPCWSTR formatter, ...)
{
    va_list args;
    va_start(args, formatter);
    wchar_t buffer[1024];
    ::vswprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), formatter, args);
    va_end(args);

    return EventLogger::GetInstance().WriteError(buffer, EVENTLOG_ERROR_TYPE);
}
