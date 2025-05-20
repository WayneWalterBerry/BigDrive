// <copyright file="ETWManifestManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

/// <summary>
/// Utility class for registering and unregistering ETW manifests with the operating system.
/// Uses wevtutil.exe to install/uninstall event provider manifests.
/// </summary>
class ETWManifestManager
{
public:

    /// <summary>
    /// Registers The Big Drive ETW manifest file with the operating system.
    /// </summary>
    /// <returns>Returns an HRESULT indicating success or failure of the operation.</returns>
    static HRESULT RegisterManifest();

    /// <summary>
    /// Unregisters a previously registered manifest.
    /// </summary>
    /// <returns>Returns an HRESULT indicating success or failure of the operation.</returns>
    static HRESULT UnregisterManifest();

    /// <summary>
    /// Registers an ETW manifest file with the operating system.
    /// </summary>
    /// <param name="manifestPath">Full path to the .man manifest file</param>
    /// <returns>S_OK if successful; otherwise, an error code</returns>
    static HRESULT RegisterManifest(LPCWSTR manifestPath);

    /// <summary>
    /// Unregisters an ETW manifest file from the operating system.
    /// </summary>
    /// <param name="manifestPath">Full path to the .man manifest file</param>
    /// <returns>S_OK if successful; otherwise, an error code</returns>
    static HRESULT UnregisterManifest(LPCWSTR manifestPath);

private:

    static HRESULT GetManifestPath(LPWSTR* ppManifestPath);

    /// <summary>
    /// Checks if the EventLog service has sufficient access to the specified file.
    /// </summary>
    /// <param name="path">Path to the file to check for permissions</param>
    /// <param name="bHasAccess">BOOL that receives TRUE if the EventLog has access, FALSE otherwise</param>
    /// <returns>S_OK if the check completed successfully; otherwise, an error code</returns>
    static HRESULT CheckEventLogAccess(LPCWSTR path, bool& bHasAccess);

    /// <summary>
    /// Grants read and execute permissions to the NT SERVICE\EventLog service for the specified file.
    /// </summary>
    /// <param name="path">Path to the file to modify permissions</param>
    /// <returns>S_OK if permissions were granted successfully; otherwise, an error code</returns>
    static HRESULT GrantEventLogAccess(LPCWSTR path);

    /// <summary>
    /// Ensures the EventLog service has read and execute access to the module DLL.
    /// The Event Log service requires these permissions to access the embedded ETW manifest 
    /// resources when the manifest is registered with wevtutil.
    /// </summary>
    /// <returns>S_OK if access check/granting was successful; E_ACCESSDENIED or another error code on failure</returns>
    static HRESULT GrantEventLogServiceAccess();

    /// <summary>
    /// Retrieves the Security Identifier (SID) for the NT SERVICE\EventLog service.
    /// This SID is used for checking and granting permissions to the EventLog service.
    /// </summary>
    /// <param name="ppSid">Pointer to receive the allocated SID. The caller must free this using FreeSid when no longer needed.</param>
    /// <returns>S_OK if the SID was retrieved successfully; otherwise, an error code</returns>
    static HRESULT GetEventLogServiceSid(PSID *ppSid);

    /// <summary>
    /// Validates that the ETW manifest was properly registered in the registry.
    /// </summary>
    /// <param name="manifestPath">Path to the ETW manifest file</param>
    /// <returns>S_OK if registry entry exists correctly; otherwise, an error code</returns>
    static HRESULT VerifyManifestRegistration(LPCWSTR manifestPath);

    /// <summary>
    /// Helper method that executes wevtutil with the specified command line.
    /// </summary>
    /// <param name="cmdLine">The full command line to execute</param>
    /// <returns>S_OK if the command executed successfully; otherwise, an error code</returns>
    static HRESULT ExecuteWevtutil(LPCWSTR cmdLine);

    /// <summary>
    /// Builds a complete wevtutil command line with the given action and manifest path.
    /// </summary>
    /// <param name="action">The action to perform ("im" for install or "um" for uninstall)</param>
    /// <param name="manifestPath">Path to the manifest file</param>
    /// <param name="buffer">Buffer to store the resulting command line</param>
    /// <param name="bufferSize">Size of the buffer in characters</param>
    /// <returns>S_OK if the command was built successfully; E_INVALIDARG if buffer is too small</returns>
    static HRESULT BuildCommandLine(LPCWSTR action, LPCWSTR manifestPath, LPWSTR buffer, SIZE_T bufferSize);
};