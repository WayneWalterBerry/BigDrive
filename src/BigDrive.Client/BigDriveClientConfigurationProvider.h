// <copyright file="BigDriveClientConfigurationProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <wtypes.h>

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
};
