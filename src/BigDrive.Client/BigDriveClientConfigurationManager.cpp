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
#include "BigDriveClientConfigurationManager.h"

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "GuidUtil.h"
#include "DriveConfiguration.h"
#include "BigDriveConfigurationClient.h"

using namespace BigDriveClient;

// Initialize the static EventLogger instance
EventLogger BigDriveClientConfigurationManager::s_eventLogger(L"BigDrive.Client");

/// <summary>
/// Retrieves the GUIDs of all drives managed by the BigDrive client.
/// </summary>
/// <param name="ppGuids">A pointer to an array of GUIDs that will be populated with the drive GUIDs.</param>
/// <param name="size">The size of the GUID array.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationManager::GetDriveGuids(GUID** ppGuids, DWORD& size)
{
    HRESULT hrReturn = S_OK;
    std::vector<LPWSTR> configurations;
    DWORD index = 0;
    WCHAR subKeyName[256];
    DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);

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
HRESULT BigDriveClientConfigurationManager::WriteDriveGuid(const GUID& guidDrive, BSTR szName, const CLSID& clsidProvider)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    HKEY hSubKey = nullptr;
    LONG result;

    if (guidDrive == GUID_NULL)
    {
        s_eventLogger.WriteError(L"WriteProviderClsId failed: guidDrive is a null pointer.");
        return E_POINTER;
    }

    if (szName == nullptr)
    {
        s_eventLogger.WriteError(L"WriteProviderClsId failed: szName is a null pointer.");
        return E_POINTER;
    }

    if (clsidProvider == GUID_NULL)
    {
        s_eventLogger.WriteError(L"WriteProviderClsId failed: clsidProvider is a null pointer.");
        return E_POINTER;
    }

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Convert the GUID to a string
    wchar_t szDriveGuid[39];
    wchar_t szProviderId[39];

    // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    hrReturn = StringFromGUID(guidDrive, szDriveGuid, ARRAYSIZE(szDriveGuid));
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to convert drive GUID to string. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = StringFromGUID(clsidProvider, szProviderId, ARRAYSIZE(szProviderId));
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to convert provider CLSID to string. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Open or create the registry key
    result = ::RegCreateKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to create or open registry key '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        goto End;
    }

    // Create a subkey for the GUID
    result = ::RegCreateKeyEx(hKey, szDriveGuid, 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to create subkey for drive GUID '%s'. Error code: 0x%08X", szDriveGuid, result);
        goto End;
    }

    // Write the GUID value to the subkey
    result = ::RegSetValueEx(hSubKey, L"Id", 0, REG_SZ, reinterpret_cast<const BYTE*>(szDriveGuid), (DWORD)((wcslen(szDriveGuid) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to write 'Id' value for drive GUID '%s'. Error code: 0x%08X", szDriveGuid, result);
        goto End;
    }

    // Write the name value to the subkey
    result = ::RegSetValueEx(hSubKey, L"Name", 0, REG_SZ, reinterpret_cast<const BYTE*>(szName), (DWORD)((wcslen(szName) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to write 'Name' value for drive GUID '%s'. Error code: 0x%08X", szDriveGuid, result);
        goto End;
    }

    // Write the CLSID value to the subkey
    result = ::RegSetValueEx(hSubKey, L"CLSID", 0, REG_SZ, reinterpret_cast<const BYTE*>(szProviderId), (DWORD)((wcslen(szProviderId) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteDriveGuid failed: Unable to write 'CLSID' value for drive GUID '%s'. Error code: 0x%08X", szDriveGuid, result);
        goto End;
    }

End:

    // Close the keys
    if (hSubKey != nullptr)
    {
        ::RegCloseKey(hSubKey);
        hSubKey = nullptr;
    }

    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }


    return hrReturn;
}

/// <summary>
/// Helper function to read drive GUID from the registry.
/// </summary>
HRESULT BigDriveClientConfigurationManager::ReadDriveGuid(GUID& guid)
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

/// <summary>
/// Reads the CLSID of a provider associated with a specific drive GUID from the registry.
/// </summary>
/// <param name="guidDrive">The GUID of the drive whose provider CLSID is to be read.</param>
/// <param name="clsidProvider">A reference to a CLSID that will be populated with the provider's CLSID.</param>
/// <returns>
/// HRESULT indicating success or failure:
/// - S_OK: The CLSID was successfully read.
/// - E_INVALIDARG: The provided GUID or CLSID string is invalid.
/// - ERROR_FILE_NOT_FOUND: the guidDrive doesn't exist.
/// - Other HRESULT values indicating specific errors during registry access or data retrieval.
/// </returns>
HRESULT BigDriveClientConfigurationManager::ReadDriveClsid(GUID guidDrive, CLSID& clsidProvider)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    DWORD index = 0;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    wchar_t clsidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    DWORD clsidStringSize = sizeof(clsidString);

    HKEY hSubKey = nullptr;

    wchar_t guidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

    // Convert the GUID to a string
    hrReturn = StringFromGUID(guidDrive, guidString, ARRAYSIZE(guidString));
    if (FAILED(hrReturn))
    {
        hrReturn = E_INVALIDARG;
        s_eventLogger.WriteErrorFormmated(L"ReadDriveClsid: Failed to convert GUID to string. GUID: %p", &guidDrive);
        return hrReturn;
    }

    // Open the registry key
    LONG result = ::RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"ReadDriveClsid: Failed to open subkey '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        goto End;
    }

    // Open the subkey for the drive GUID
    result = ::RegOpenKeyEx(hKey, guidString, 0, KEY_READ, &hSubKey);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"ReadDriveClsid: Failed to open subkey '%s'. Error code: 0x%08X", guidString, result);
        return hrReturn;
    }

    // Read the CLSID value from the subkey
    result = ::RegQueryValueEx(hSubKey, L"CLSID", nullptr, nullptr, reinterpret_cast<LPBYTE>(clsidString), &clsidStringSize);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"ReadDriveClsid: Failed to read CLSID value from subkey '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        goto End;
    }

    // Convert the CLSID string to a GUID
    hrReturn = GUIDFromString(clsidString, &clsidProvider);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"ReadDriveClsid: Failed to convert CLSID string '%s' to GUID. HRESULT: 0x%08X", clsidString, hrReturn);
        goto End;
    }

End:

    // Close the registry key
    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    if (hSubKey != nullptr)
    {
        ::RegCloseKey(hSubKey);
        hSubKey = nullptr;
    }

    return hrReturn;
}

/// <summary>
/// Deletes all registry subkeys where drive GUIDs are stored under the "Software\BigDrive\Drives" registry path.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationManager::DeleteAllDriveGuids()
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    LONG result;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Open the registry key
    result = ::RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        // If the key doesn't exist, return success
        if (result == ERROR_FILE_NOT_FOUND)
        {
            hrReturn = S_OK;
            s_eventLogger.WriteError(L"DeleteAllDriveGuids: Registry key does not exist. No action needed.");
            goto End;
        }

        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"DeleteAllDriveGuids: Failed to open registry key '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        goto End;
    }

    // Delete all subkeys for the GUID
    result = ::RegDeleteTree(hKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"DeleteAllDriveGuids: Failed to delete all subkeys under '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        goto End;
    }

    s_eventLogger.WriteInfo(L"DeleteAllDriveGuids: Successfully deleted all drive GUID subkeys.");

End:

    // Close the registry key
    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    return hrReturn;
}

/// <summary>
/// Deletes a specific registry subkey corresponding to the provided drive GUID
/// under the "Software\BigDrive\Drives" registry path.
/// </summary>
/// <param name="guid">The GUID of the drive to delete.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationManager::DeleteDriveGuid(const GUID& guid)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    LONG result;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

    // Convert the GUID to a string
    wchar_t guidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
    {
        hrReturn = E_INVALIDARG;
        s_eventLogger.WriteErrorFormmated(L"DeleteDriveGuid: Failed to convert GUID to string. GUID: %p", &guid);
        goto End;
    }

    // Open the registry key
    result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_FILE_NOT_FOUND)
        {
            hrReturn = S_OK; // If the key doesn't exist, treat it as success
            s_eventLogger.WriteErrorFormmated(L"DeleteDriveGuid: Registry key '%s' does not exist. No action needed.", drivesRegistryPath.c_str());
            goto End;
        }

        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"DeleteDriveGuid: Failed to open registry key '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
        goto End;
    }

    // Delete the subkey for the GUID
    result = RegDeleteKey(hKey, guidString);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_FILE_NOT_FOUND)
        {
            hrReturn = S_OK; // If the subkey doesn't exist, treat it as success
            s_eventLogger.WriteErrorFormmated(L"DeleteDriveGuid: Subkey for GUID '%s' does not exist. No action needed.", guidString);
            goto End;
        }

        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"DeleteDriveGuid: Failed to delete subkey for GUID '%s'. Error code: 0x%08X", guidString, result);
        goto End;
    }

    s_eventLogger.WriteInfo(L"DeleteDriveGuid: Successfully deleted subkey for GUID.");

End:

    // Close the registry key
    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    return hrReturn;
}

/// <summary>
/// Retrieves the CLSIDSs of all providers registered for the BigDrive client.
/// </summary>
/// <param name="ppGuids">A pointer to an array of CLSIDs that will be populated with the provider CLSIDs.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationManager::GetProviderClsIds(CLSID** ppClisd)
{
    HRESULT hrReturn = S_OK;
    std::vector<LPWSTR> configurations;
    DWORD index = 0;
    WCHAR subKeyName[256];
    DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
    DWORD size;

    if (!ppClisd)
    {
        s_eventLogger.WriteError(L"GetProviderClsIds failed: ppGuids is a null pointer.");
        return E_POINTER; // Invalid pointer
    }

    *ppClisd = nullptr;

    // Define the registry path
    const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Providers";

    // Open the registry key
    HKEY hKey = nullptr;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        s_eventLogger.WriteErrorFormmated(L"GetProviderClsIds failed: Unable to open registry key '%s'. Error code: 0x%08X", drivesRegistryPath.c_str(), result);
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
            s_eventLogger.WriteError(L"GetProviderClsIds failed: Out of memory while allocating subkey name.");
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

    *ppClisd = (GUID*)::CoTaskMemAlloc(size * sizeof(GUID));
    if (!*ppClisd)
    {
        s_eventLogger.WriteError(L"GetProviderClsIds failed: Out of memory while allocating GUID array.");
        hrReturn = E_OUTOFMEMORY;
        goto End;
    }

    // Copy the strings to the output array
    for (DWORD i = 0; i < size; ++i)
    {
        if (SUCCEEDED(GUIDFromString(configurations[i], &(*ppClisd)[i])) == FALSE)
        {
            s_eventLogger.WriteErrorFormmated(L"GetProviderClsIds failed: Invalid GUID format for subkey '%s'.", configurations[i]);
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
/// Checks if a provider subkey exists in the "Software\\BigDrive\\Providers" registry path.
/// </summary>
/// <param name="clsidProvider">The CLSID of the provider to check.</param>
/// <returns>HRESULT indicating success or failure. S_OK if the subkey exists, S_FALSE if it does not exist.</returns>
HRESULT BigDriveClientConfigurationManager::DoesProviderSubkeyExist(const CLSID& clsidProvider)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    HKEY hSubKey = nullptr;
    LONG result;

    if (clsidProvider == GUID_NULL)
    {
        s_eventLogger.WriteError(L"WriteProviderClsId failed: clsidProvider is a null pointer.");
        return E_POINTER;
    }

    // Define the registry path
    const wchar_t providersRegistryPath[] = L"Software\\BigDrive\\Providers";

    // Convert the CLSID to a string
    wchar_t clsidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    hrReturn = StringFromGUID(clsidProvider, clsidString, ARRAYSIZE(clsidString));
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"DoesProviderSubkeyExist: Failed to convert CLSID to string. CLSID: %p", &clsidProvider);
        return hrReturn;
    }

    // Open the registry key
    result = ::RegOpenKeyEx(HKEY_CURRENT_USER, providersRegistryPath, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_FILE_NOT_FOUND)
        {
            s_eventLogger.WriteInfo(L"DoesProviderSubkeyExist: Registry path does not exist. No subkeys to check.");
            return S_FALSE; // Registry path does not exist
        }

        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"DoesProviderSubkeyExist: Failed to open registry key '%s'. Error code: 0x%08X", providersRegistryPath, result);
        goto End;
    }

    // Check if the subkey for the CLSID exists

    result = ::RegOpenKeyEx(hKey, clsidString, 0, KEY_READ, &hSubKey);
    if (result == ERROR_SUCCESS)
    {
        hrReturn = S_OK;
        goto End;
    }
    else if (result == ERROR_FILE_NOT_FOUND)
    {
        // Subkey does not exist
        s_eventLogger.WriteErrorFormmated(L"DoesProviderSubkeyExist: Subkey for CLSID '%s' does not exist.", clsidString);
        hrReturn = S_FALSE;
        goto End;
    }
    else
    {
        // Other error
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"DoesProviderSubkeyExist: Failed to check subkey for CLSID '%s'. Error code: 0x%08X", clsidString, result);
        goto End;
    }

End:

    if (hSubKey != nullptr)
    {
        ::RegCloseKey(hSubKey);
        hSubKey = nullptr;
    }

    // Close the registry key
    if (hKey)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    return hrReturn;
}

/// <summary>
/// Writes a registry key for a provider CLSID under the "Software\BigDrive\Providers" registry path.
/// </summary>
/// <param name="clsidProvider">The CLSID of the provider to write as a registry key.</param>
/// <param name="szName">The display name of the provider.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationManager::WriteProviderClsId(const CLSID& clsidProvider, BSTR szName)
{
    HRESULT hrReturn = S_OK;
    HKEY hKey = nullptr;
    HKEY hSubKey = nullptr;
    LONG result;

    if (clsidProvider == GUID_NULL)
    {
        s_eventLogger.WriteError(L"WriteProviderClsId failed: clsidProvider is a null pointer.");
        return E_POINTER;
    }

    if (szName == nullptr)
    {
        s_eventLogger.WriteError(L"WriteProviderClsId failed: szName is a null pointer.");
        return E_POINTER;
    }

    // Define the registry path
    const std::wstring providersRegistryPath = L"Software\\BigDrive\\Providers";

    // Convert the CLSID to a string
    wchar_t szProviderId[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

    hrReturn = StringFromGUID(clsidProvider, szProviderId, ARRAYSIZE(szProviderId));
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"WriteProviderClsId failed: Unable to convert provider CLSID to string. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Open or create the registry key
    result = ::RegCreateKeyEx(HKEY_CURRENT_USER, providersRegistryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteProviderClsId failed: Unable to create or open registry key '%s'. Error code: 0x%08X", providersRegistryPath.c_str(), result);
        goto End;
    }

    // Create a subkey for the CLSID
    result = ::RegCreateKeyEx(hKey, szProviderId, 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteProviderClsId failed: Unable to create subkey for provider CLSID '%s'. Error code: 0x%08X", szProviderId, result);
        goto End;
    }

    // Write the CLSID value to the subkey
    result = ::RegSetValueEx(hSubKey, L"Id", 0, REG_SZ, reinterpret_cast<const BYTE*>(szProviderId), (DWORD)((wcslen(szProviderId) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteProviderClsId failed: Unable to write 'Id' value for provider CLSID '%s'. Error code: 0x%08X", szProviderId, result);
        goto End;
    }

    // Write the name value to the subkey
    result = ::RegSetValueEx(hSubKey, L"Name", 0, REG_SZ, reinterpret_cast<const BYTE*>(szName), (DWORD)((wcslen(szName) + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS)
    {
        hrReturn = HRESULT_FROM_WIN32(result);
        s_eventLogger.WriteErrorFormmated(L"WriteProviderClsId failed: Unable to write 'Name' value for provider CLSID '%s'. Error code: 0x%08X", szProviderId, result);
        goto End;
    }

End:
    // Close the keys
    if (hSubKey != nullptr)
    {
        ::RegCloseKey(hSubKey);
        hSubKey = nullptr;
    }

    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
        hSubKey = nullptr;
    }

    return hrReturn;
}

/// <summary>
/// Check all Registered Drives and remove any that have a provider CLSID that doesn't exist.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveClientConfigurationManager::CleanDrives()
{
    HRESULT hrReturn = S_OK;
    DWORD dwSize;
    GUID* pGuids = nullptr;

    // Get the drive GUIDs
    hrReturn = GetDriveGuids(&pGuids, dwSize);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"CleanDrives failed: Unable to get drive GUIDs. HRESULT: 0x%08X", hrReturn);
        return hrReturn;
    }

    // Iterate through all GUIDs and call ReadDriveClsid for each
    for (DWORD i = 0; i < dwSize; ++i)
    {
        CLSID clsid;
        hrReturn = ReadDriveClsid(pGuids[i], clsid);
        switch (hrReturn)
        {
        case S_OK:

            // If the CLSID Exists Then The Drive Registration Is Valid. Now
            // chck that the Provider is registered.
            hrReturn = DoesProviderSubkeyExist(clsid);
            switch (hrReturn)
            {
            case S_OK:
                // The CLSID exists, so the drive registration is valid.
                break;
            case S_FALSE:
                // The CLSID does not exist, so delete the drive GUID.
                hrReturn = DeleteDriveGuid(pGuids[i]);
                if (FAILED(hrReturn))
                {
                    s_eventLogger.WriteErrorFormmated(L"CleanDrives failed: Unable to delete drive GUID at index %d. HRESULT: 0x%08X", i, hrReturn);
                    goto End;
                }
                break;
            default:
                // Handle failure cases
                s_eventLogger.WriteErrorFormmated(L"CleanDrives failed: Unable to check provider subkey existence for CLSID at index %d. HRESULT: 0x%08X", i, hrReturn);
                goto End;
            }

            break;

        case S_FALSE:
            // The The Drive doesn't exist, there is nothing to do
            s_eventLogger.WriteErrorFormmated(L"CleanDrives failed: Unable to find the drive GUID at index %d. HRESULT: 0x%08X", hrReturn);
            break;

        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
            s_eventLogger.WriteErrorFormmated(L"CleanDrives failed: Unable to find the drive GUID at index %d. HRESULT: 0x%08X", hrReturn);
            continue;

        default:
            // Handle failure cases
            s_eventLogger.WriteErrorFormmated(L"CleanDrives failed: Unable to check that the drive GUID at index %d. HRESULT: 0x%08X", i, hrReturn);
            goto End;
        }
    }

End:

    // Clean up
    if (pGuids != nullptr)
    {
        ::CoTaskMemFree(pGuids);
        nullptr == nullptr;
    }

    return hrReturn;
}
