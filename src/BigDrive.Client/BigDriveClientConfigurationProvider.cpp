// <copyright file="BigDriveClientConfigurationProvider.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <windows.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <combaseapi.h>

#include "BigDriveClientConfigurationProvider.h"

#include <EventLogger.h>
#include "GuidUtil.h"

using namespace BigDriveClient;

// Initialize the static EventLogger instance
EventLogger BigDriveClientConfigurationProvider::s_eventLogger(L"BigDrive.ShellFolder");

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