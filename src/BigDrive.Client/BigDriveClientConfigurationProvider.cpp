// <copyright file="BigDriveClientConfigurationProvider.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <combaseapi.h>

// Header
#include "BigDriveClientConfigurationProvider.h"

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "GuidUtil.h"

using namespace BigDriveClient;

// Initialize the static EventLogger instance
EventLogger BigDriveClientConfigurationProvider::s_eventLogger(L"BigDrive.Client");

/// <summary>
/// Retrieves the GUIDs of all drives managed by the BigDrive client.
/// </summary>
/// <param name="ppGuids">A pointer to an array of GUIDs that will be populated with the drive GUIDs.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationProvider::GetDriveGuids(GUID** ppGuids)
{
    HRESULT hrReturn = S_OK;
    std::vector<LPWSTR> configurations;
    DWORD index = 0;
    WCHAR subKeyName[256];
    DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
    DWORD size;

    if (!ppGuids)
    {
        s_eventLogger.WriteError(L"GetDriveGuids failed: ppGuids is a null pointer.");
        return E_POINTER; // Invalid pointer
    }

    *ppGuids = nullptr;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Open the registry key
    HKEY hKey = nullptr;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        s_eventLogger.WriteErrorFormmated(L"GetDriveGuids failed: Unable to open registry key '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    // Enumerate subkeys
    while (RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        // Allocate memory for the subkey name and add it to the vector
        LPWSTR subKeyCopy = (LPWSTR)::CoTaskMemAlloc((subKeyNameSize + 1) * sizeof(WCHAR));
        if (!subKeyCopy)
        {
            s_eventLogger.WriteError(L"GetDriveGuids failed: Out of memory while allocating subkey name.");
            ::RegCloseKey(hKey);
            return E_OUTOFMEMORY;
        }

        wcscpy_s(subKeyCopy, subKeyNameSize + 1, subKeyName);
        configurations.push_back(subKeyCopy);

        // Reset subKeyNameSize and increment index
        subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
        ++index;
    }

    // Allocate the output array
    size = static_cast<DWORD>(configurations.size());

    *ppGuids = (GUID*)::CoTaskMemAlloc(size * sizeof(GUID));
    if (!*ppGuids)
    {
        s_eventLogger.WriteError(L"GetDriveGuids failed: Out of memory while allocating GUID array.");
        hrReturn = E_OUTOFMEMORY;
        goto End;
    }

    // Copy the strings to the output array
    for (DWORD i = 0; i < size; ++i)
    {
        if (SUCCEEDED(GUIDFromString(configurations[i], &(*ppGuids)[i])) == FALSE)
        {
            s_eventLogger.WriteErrorFormmated(L"GetDriveGuids failed: Invalid GUID format for subkey '%s'.", configurations[i]);
            hrReturn = E_INVALIDARG;
            goto End;
        }
    }

End:

    if (hKey)
    {
        ::RegCloseKey(hKey);
    }

    for (LPWSTR str : configurations)
    {
        ::CoTaskMemFree(str);
    }

    return hrReturn;
}

/// <summary>
/// Writes a registry key for a drive GUID under the "Software\BigDrive\Drives" registry path.
/// </summary>
/// <param name="guid">The GUID of the drive to write as a registry key.</param>
/// <param name="clsdid">The CLSID of the drive to write as a registry key.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationProvider::WriteDriveGuid(const GUID& guidDrive, BSTR szName, const CLSID& clsidProvider)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    HKEY hSubKey = nullptr;
    LONG result;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Convert the GUID to a string
    wchar_t szDriveGuid[39];
    wchar_t szProviderId[39];

    // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    hrReturn = StringFromGUID(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
    if (FAILED(hrReturn))
    {
        goto End;
    }

    // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    hrReturn = StringFromGUID(clsidProvider, szProviderId, ARRAYSIZE(szProviderId));
    if (FAILED(hrReturn))
    {
        goto End;
    }

    // Open or create the registry key
    result = ::RegCreateKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    // Create a subkey for the GUID
    result = ::RegCreateKeyEx(hKey, szDriveGuid, 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    // Write the GUID value to the subkey
    result = ::RegSetValueEx(hSubKey, L"Id", 0, REG_SZ, reinterpret_cast<const BYTE*>(szDriveGuid), (DWORD)((wcslen(szDriveGuid) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    /// Write the name value to the subkey
    result = ::RegSetValueEx(hSubKey, L"Name", 0, REG_SZ, reinterpret_cast<const BYTE*>(szName), (DWORD)((wcslen(szName) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    /// Write the name value to the subkey
    result = ::RegSetValueEx(hSubKey, L"CLSID", 0, REG_SZ, reinterpret_cast<const BYTE*>(szProviderId), (DWORD)((wcslen(szProviderId) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

End:

    // Close the keys
    if (hSubKey != nullptr)
    {
        ::RegCloseKey(hSubKey);
    }

    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
    }

    return S_OK;
}

/// <summary>
/// Helper function to read drive GUID from the registry.
/// </summary>
HRESULT BigDriveClientConfigurationProvider::ReadDriveGuid(GUID& guid)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    DWORD index = 0;
    WCHAR subKeyName[256];
    DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Open the registry key
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    // Enumerate the first subkey (assuming only one GUID is stored)
    result = RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    // Convert the subkey name (GUID string) back to a GUID
    hrReturn = GUIDFromString(subKeyName, &guid);
    if (FAILED(hrReturn))
    {
        hrReturn = E_INVALIDARG; // Failed to parse GUID
        goto End;
    }

End:

    // Close the registry key
    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
    }

    return hrReturn;
}

HRESULT BigDriveClientConfigurationProvider::DeleteDriveGuid(const GUID& guid)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    LONG result;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Convert the GUID to a string
    wchar_t guidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    hrReturn = StringFromGUID(guid, guidString, ARRAYSIZE(guidString));
    {
        // Failed to convert GUID to string
        goto End;
    }

    // Open the registry key
    result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        // If the key doesn't exist, return success
        if (result == ERROR_FILE_NOT_FOUND)
        {
            hrReturn = S_OK;
            goto End;
        }

        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

    // Delete the subkey for the GUID
    result = RegDeleteKey(hKey, guidString);
    if (result != ERROR_SUCCESS)
    {
        // If the subkey doesn't exist, return success
        if (result == ERROR_FILE_NOT_FOUND)
        {
            hrReturn = S_OK;
            goto End;
        }

        hrReturn = HRESULT_FROM_WIN32(result);
        goto End;
    }

End:

    // Close the registry key
    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
    }

    return hrReturn;
}
