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
    static HRESULT RegisterManifest();

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