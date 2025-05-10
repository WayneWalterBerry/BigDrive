// <copyright file="BigDriveClientConfigurationProvider.h" company="Wayne Walter Berry">
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
class BigDriveClientConfigurationProvider
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
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT GetDriveGuids(GUID** ppGuids);

    /// <summary>
    /// Writes a registry key for a drive GUID under the "Software\BigDrive\Drives" registry path.
    /// </summary>
    /// <param name="driveGuid">The GUID of the drive to write as a registry key.</param>
    /// <param name="szName">The display name of the drive.</param>
    /// <param name="providerGuid">The GUID of the provider.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    static HRESULT WriteDriveGuid(const GUID& guidDrive, BSTR szName, const CLSID& clsidProvider);

    static HRESULT ReadDriveGuid(GUID& guid);

    static HRESULT DeleteDriveGuid(const GUID& guid);
};
