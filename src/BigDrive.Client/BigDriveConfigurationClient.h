// <copyright file="BigDriveConfigurationClient.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <string>
#include <wtypes.h>

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "DriveConfiguration.h"

class BigDriveConfigurationClient
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

public:

    /// <summary>
    /// Gets the configuration from the registry by calling the BigDriveConfiguration COM object.
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="pszConfiguration">Configuration</param>
    static HRESULT GetDriveConfiguration(GUID guid, LPWSTR* pszConfiguration);

    /// <summary>
    /// Gets the configuration from the registry by calling the BigDriveConfiguration COM object.
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="pDriveConfiguration">Configuration</param>
    static HRESULT GetDriveConfiguration(GUID guid, DriveConfiguration& driveConfiguration);

};