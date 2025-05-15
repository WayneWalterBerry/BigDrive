// <copyright file="RegistrationManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <CommCtrl.h>
#include <guiddef.h>

// Local
#include "DriveConfiguration.h"

// Shared
#include "..\Shared\EventLogger.h"

class __declspec(dllexport) RegistrationManager
{
private:

    static EventLogger s_eventLogger;

public:

    /// <summary>
    /// Enumerates all registered drive GUIDs from the registry, retrieves their configuration,
    /// and registers each as a shell folder in Windows Explorer. For each drive, this method
    /// obtains its configuration, then creates the necessary registry entries to expose the
    /// drive as an IShellFolder. Logs errors and informational messages for each operation.
    /// Returns S_OK if all drives are registered successfully, or an error HRESULT if any step fails.
    /// </summary>
    static HRESULT RegisterShellFoldersFromRegistry();

    /// <summary>
    /// Reads the Registry to get the CLSIDs of all registered shell folders.
    /// </summary>
    /// <param name="ppClsids">Return the CLSIDS to use for ShellFolders</param>
    /// <returns>HRESULT indicating success or failure</returns>
    static HRESULT GetRegisteredCLSIDs(CLSID** ppClsids, DWORD& dwSize);

    /// <summary>
    /// Register the shell folder with the given drive Guid.
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="bstrName">Display name</param>
    /// <returns>HRESULT indicating success or failure</returns>
    static HRESULT RegisterShellFolder(GUID guidDrive, BSTR bstrName);

    static HRESULT GetModuleFileNameW(LPWSTR szModulePath, DWORD dwSize);

    static HRESULT UnregisterShellFolders();

private:

    /// <summary>
    /// Gets the configuration from the registry by calling the BigDriveConfiguration COM object.
    /// </summary>
    static HRESULT GetConfiguration(GUID guid, DriveConfiguration& driveConfiguration);

    /// <summary>
    /// Unregister the shell folder with the given GUID.
    /// </summary>
    /// <param name="guid">Drive GUID</param>
    /// <returns>HRESULT indicating success or failure</returns>
    static HRESULT UnregisterShellFolder(GUID guid);

    /// <summary>
    /// Logs a formatted info message with the Drive Guid
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="formatter">The format string for the error message.</param>
    /// <param name="...">The arguments for the format string.</param>
    /// <returns>HRESULT indicating success or failure of the logging operation.</returns>
    static HRESULT WriteInfoFormmated(GUID guid, LPCWSTR formatter, ...);

    /// <summary>
    /// Logs a error message with the Drive Guid
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="message">The format string for the error message.</param>
    /// <returns>HRESULT indicating success or failure of the logging operation.</returns>
    static HRESULT WriteError(GUID guid, LPCWSTR message);

    /// <summary>
    /// Logs a formatted error message with the Drive Guid
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    /// <param name="formatter">The format string for the error message.</param>
    /// <param name="...">The arguments for the format string.</param>
    /// <returns>HRESULT indicating success or failure of the logging operation.</returns>
    static HRESULT WriteErrorFormmated(GUID guid, LPCWSTR formatter, ...);
};
