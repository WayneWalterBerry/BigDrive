// <copyright file="RegistrationManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <CommCtrl.h>
#include <guiddef.h>

#include "DriveConfiguration.h"
#include <EventLogger.h>

class RegistrationManager
{
private:

    static EventLogger s_eventLogger;

public:

    /// <summary>
    /// Provides access to the singleton instance of RegistrationManager.
    /// </summary>
    static RegistrationManager& GetInstance();

    HRESULT RegisterShellFoldersFromRegistry();

    HRESULT GetRegisteredCLSIDs(CLSID** ppClsids);

private:

    // Private constructor to prevent direct instantiation
    RegistrationManager() = default;

    // Delete copy constructor and assignment operator to prevent copying
    RegistrationManager(const RegistrationManager&) = delete;
    RegistrationManager& operator=(const RegistrationManager&) = delete;

    /// <summary>
    /// Gets the configuration from the registry by calling the BigDriveConfiguration COM object.
    /// </summary>
    HRESULT GetConfiguration(GUID guid, DriveConfiguration& driveConfiguration);

    /// <summary>
    /// Register the shell folder with the given GUID.
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="bstrName">Display name</param>
    HRESULT RegisterShellFolder(GUID guid, BSTR bstrName);

    /// <summary>
    /// Unregister the shell folder with the given GUID.
    /// </summary>
    /// <param name="guid">Drive GUID</param>
    /// <returns>HRESULT indicating success or failure</returns>
    HRESULT UnregisterShellFolder(GUID guid);

    /// <summary>
    /// Write a formatted error message to the Event Viewer
    /// </summary>
    /// <param name="formatter">printf style formatter</param>
    HRESULT WriteError(LPCWSTR formatter, ...);

    /// <summary>
    /// Write a formatted info message to the Event Viewer
    /// </summary>
    /// <param name="formatter">printf style formatter</param>
    HRESULT WriteInfo(LPCWSTR formatter, ...);
};
