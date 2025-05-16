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

    /// <summary>
    /// Checks whether the bitness (32-bit or 64-bit) of the current DLL matches the bitness of the operating system.
    /// This method retrieves the current module's file path, determines its bitness, and compares it to the OS bitness.
    /// </summary>
    /// <param name="isMatch">[out] Set to true if the DLL and OS bitness match, false otherwise. Unchanged on failure.</param>
    /// <returns>
    /// S_OK if the check succeeds and isMatch is set;
    /// or an error HRESULT if the DLL file cannot be inspected or the OS bitness cannot be determined.
    /// </returns>
    static HRESULT CheckDllAndOSBitnessMatch(bool& isMatch);

    /// <summary>
    /// Scans all CLSID entries in the Windows registry and removes those associated with BigDrive shell folders.
    /// This method identifies shell folders registered by BigDrive by checking the InprocServer32 path for the
    /// "BigDrive.ShellFolder" substring, then unregisters and deletes their related registry keys. Returns S_OK
    /// if cleanup succeeds, or an error HRESULT if any step fails.
    /// </summary>
    static HRESULT CleanUpShellFolders();

private:

    /// <summary>
    /// Registers the DefaultIcon registry entry for the specified drive shell extension CLSID,
    /// setting it to use the standard Windows file system drive icon. This ensures that the
    /// shell extension appears with the same icon as local drives in File Explorer.
    /// </summary>
    /// <param name="guidDrive">The CLSID of the drive shell folder to register the icon for.</param>
    /// <returns>HRESULT indicating success or failure of the registration operation.</returns>
    static HRESULT RegisterDefaultIcon(GUID guidDrive);

    /// <summary>
    /// Registers the COM in-process server for the specified drive shell folder CLSID under
    /// HKEY_CLASSES_ROOT\CLSID\{guid}\InprocServer32. This method sets the default value to the
    /// module path of the DLL and configures the ThreadingModel as "Apartment". It also creates
    /// the required Implemented Categories subkey for CATID_ShellFolder
    /// ({00021490-0000-0000-C000-000000000046}), which identifies the object as a shell folder
    /// extension. All registry operations use WCHAR strings and include error handling.
    /// </summary>
    /// <param name="guidDrive">The CLSID of the drive shell folder to register.</param>
    /// <param name="bstrName">The display name of the drive shell folder.</param>
    /// <returns>HRESULT indicating success or failure of the registration operation.</returns>
    static HRESULT RegisterInprocServer32(GUID guidDrive, BSTR bstrName);

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

    /// <summary>
    /// Determines whether the specified DLL is compiled as a 64-bit or 32-bit binary.
    /// This method inspects the PE headers of the file at the given path and sets the output parameter accordingly.
    /// </summary>
    /// <param name="dllPath">The full path to the DLL file to inspect.</param>
    /// <param name="is64Bit">[out] Set to true if the DLL is 64-bit, false if 32-bit. Unchanged on failure.</param>
    /// <returns>
    /// S_OK if the check succeeds and is64Bit is set;
    /// HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if the file does not exist;
    /// or another error HRESULT if the file cannot be read or is not a valid PE file.
    /// </returns>
    static HRESULT IsDll64Bit(const wchar_t* dllPath, bool& is64Bit);

    /// <summary>
    /// Determines whether the current operating system is 64-bit or 32-bit.
    /// This method queries the system processor architecture and sets the output parameter accordingly.
    /// </summary>
    /// <param name="is64Bit">[out] Set to true if the OS is 64-bit, false if 32-bit. Unchanged on failure.</param>
    /// <returns>
    /// S_OK if the check succeeds and is64Bit is set;
    /// E_FAIL if the processor architecture is unknown or unsupported.
    /// </returns>
    static HRESULT IsCurrentOS64Bit(bool& is64Bit);
};
