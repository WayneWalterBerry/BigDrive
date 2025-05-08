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
        DriveConfiguration driveConfiguration;

        // Get the configuration for the drive from the COM++ BigDrive.Service
        hrReturn = GetConfiguration(pGuids[index], driveConfiguration);
        if (FAILED(hrReturn))
        {
            break;
        }

        hrReturn = RegisterShellFolder(pGuids[index], driveConfiguration.name);
        if (FAILED(hrReturn))
        {
            break;
        }

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
        return E_FAIL; // Failed to convert GUID to string
    }

    // Get the full path of the module
    if (!GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), modulePath, MAX_PATH))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Register CLSID_BigDriveShellFolder

    clsidPath = L"CLSID\\" + std::wstring(guidString) + L"\\InprocServer32";
    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        hrReturn = E_FAIL;
        goto End;
    }

    ::RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(modulePath), static_cast<DWORD>((wcslen(modulePath) + 1) * sizeof(wchar_t)));
    ::RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Apartment"), sizeof(L"Apartment"));

    if (hKey)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    // Register as a Drive (directly as a ShellFolder) - CHANGED TO USE HKEY_CURRENT_USER
    namespacePath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\" + std::wstring(guidString);
    if (RegCreateKeyExW(HKEY_CURRENT_USER, namespacePath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        hrReturn = E_FAIL;
        goto End;
    }

    // Set a default value (display name for the drive)
    if (RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(bstrName), sizeof(bstrName)) != ERROR_SUCCESS)
    {
        hrReturn = E_FAIL;
        goto End;
    }

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = nullptr;
    }

    if (RegCreateKeyExW(HKEY_CLASSES_ROOT, componentCategoryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
    {
        hrReturn = E_FAIL;
        goto End;
    }

    // Add your CLSID as a subkey under "Implementations"
    if (RegCreateKeyExW(hKey, guidString, 0, nullptr, 0, KEY_WRITE, nullptr, &hClsidKey, nullptr) != ERROR_SUCCESS)
    {
        hrReturn = E_FAIL;
        goto End;
    }

End:

    if (hClsidKey)
    {
        RegCloseKey(hClsidKey);
        hClsidKey = nullptr;
    }

    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = nullptr;
    }

    return hrReturn;
}
