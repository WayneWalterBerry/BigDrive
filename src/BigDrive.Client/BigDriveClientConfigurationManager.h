// <copyright file="BigDriveClientConfigurationManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <wtypes.h>

// Shared
#include "..\Shared\EventLogger.h"

/// <summary>
/// Provides functionality to interact with the BigDrive client configuration,
/// including retrieving drive GUIDs.
/// </summary>
class BigDriveClientConfigurationManager
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

public:

    /// <summary>
    /// Retrieves the GUIDs of all drives managed by the BigDrive client.
    /// </summary>
    /// <param name="ppGuids">A pointer to an array of GUIDs that will be populated with the drive GUIDs.</param>
    /// <param name="size">The size of the GUID array.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT GetDriveGuids(GUID** ppGuids, DWORD& size);

    /// <summary>
    /// Writes a registry key for a drive GUID under the "Software\BigDrive\Drives" registry path.
    /// </summary>
    /// <param name="driveGuid">The GUID of the drive to write as a registry key.</param>
    /// <param name="szName">The display name of the drive.</param>
    /// <param name="providerGuid">The GUID of the provider.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT WriteDriveGuid(const GUID& guidDrive, BSTR szName, const CLSID& clsidProvider);

    /// <summary>
    /// Helper function to read drive GUID from the registry.
    /// </summary>
    static HRESULT ReadDriveGuid(GUID& guid);

    /// <summary>
    /// Deletes a specific registry subkey corresponding to the provided drive GUID
    /// under the "Software\BigDrive\Drives" registry path.
    /// </summary>
    /// <param name="guid">The GUID of the drive to delete.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT DeleteDriveGuid(const GUID& guid);

    /// <summary>
    /// Deletes all registry subkeys where drive GUIDs are stored under the "Software\BigDrive\Drives" registry path.
    /// </summary>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT DeleteAllDriveGuids();

    /// <summary>
    /// Retrieves the CLSIDSs of all providers registered for the BigDrive client.
    /// </summary>
    /// <param name="ppGuids">A pointer to an array of CLSIDs that will be populated with the provider CLSIDs.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT GetProviderClsIds(CLSID** ppClisd);

    /// <summary>
    /// Writes a registry key for a provider CLSID under the "Software\BigDrive\Providers" registry path.
    /// </summary>
    /// <param name="driveGuid">The GUID of the drive to write as a registry key.</param>
    /// <param name="szName">The display name of the drive.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT WriteProviderClsId(const CLSID& clsidProvider, BSTR szName);

    static HRESULT ReadDriveClsid(GUID guidDrive, CLSID& clsidProvider);

    /// <summary>
    /// Checks if a provider subkey exists in the "Software\\BigDrive\\Providers" registry path.
    /// </summary>
    /// <param name="clsidProvider">The CLSID of the provider to check.</param>
    /// <returns>HRESULT indicating success or failure. S_OK if the subkey exists, S_FALSE if it does not exist.</returns>
    static HRESULT DoesProviderSubkeyExist(const CLSID& clsidProvider);

    static HRESULT CleanDrives();
};
