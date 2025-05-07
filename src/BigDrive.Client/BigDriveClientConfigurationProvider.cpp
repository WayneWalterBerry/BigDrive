// <copyright file="BigDriveClientConfigurationProvider.h" company="Wayne Walter Berry">
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
#include "GuidUtil.h"

using namespace BigDriveClient;

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
        return E_POINTER; // Invalid pointer
    }

    *ppGuids = nullptr;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Open the registry key
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
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
        hrReturn = E_OUTOFMEMORY;
        goto End;
    }

    // Copy the strings to the output array
    for (DWORD i = 0; i < size; ++i)
    {
        if (SUCCEEDED(GUIDFromString(configurations[i], &(*ppGuids)[i])) == FALSE)
        {
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
