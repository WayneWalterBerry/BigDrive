// <copyright file="RegistrationManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <CommCtrl.h>
#include <guiddef.h>

// Local
#include "..\BigDrive.Client\DriveConfiguration.h"
#include "BigDriveShellFolderEventLogger.h"

/// <summary>
/// Provides a set of static methods for registering, unregistering, and managing custom shell folder extensions
/// in the Windows registry for the BigDrive application. This class handles the creation and removal of all
/// necessary registry entries to expose virtual drives as IShellFolder objects in Windows Explorer, including
/// COM registration, namespace integration, icon registration, and component category management.
///
/// Key responsibilities include:
/// - Enumerating and registering all configured drives as shell folders, including error handling and logging.
/// - Creating and removing registry keys under HKEY_CLASSES_ROOT and HKEY_CURRENT_USER to integrate drives
///   into Explorer's namespace and COM infrastructure.
/// - Managing security and permissions for protected registry keys, including taking ownership and granting
///   full control to the current user when required by OS restrictions.
/// - Registering and unregistering default icons and COM in-process servers for each drive shell folder.
/// - Providing utility methods for bitness checks, privilege elevation, and SID retrieval to support
///   secure and robust registry operations.
/// - Logging all significant operations and errors to the Windows Event Viewer for diagnostics.
///
/// This class is designed for use by installer, configuration, and maintenance tools that need to
/// programmatically manage the lifecycle of BigDrive shell folder extensions on Windows systems.
/// All methods are static and thread-safe for use in multi-threaded scenarios.
/// </summary>
class RegistrationManager
{
private:

    static BigDriveShellFolderEventLogger s_eventLogger;

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
    /// Retrieves the full path of the current module (DLL or EXE) associated with the BigDrive shell extension.
    /// This method wraps the Windows API GetModuleFileNameW, using the module handle for the current image,
    /// and writes the result to the provided buffer. Returns S_OK on success, or an HRESULT error code if the
    /// path cannot be retrieved.
    /// </summary>
    static HRESULT GetModuleFileNameW(LPWSTR szModulePath, DWORD dwSize);

    /// <summary>
    /// Takes ownership and grants full control of the specified registry key to the current user.
    /// </summary>
    /// <returns>S_OK on success, or an HRESULT error code on failure.</returns>
	/// <remarks>
    /// <summary>
    /// CLSID {00021493-0000-0000-C000-000000000046} is linked to Windows Shell Link objects,
    /// used for shortcuts. OS restrictions enforce COM security to ensure only authorized
    /// processes interact with them, preventing unauthorized access and potential exploits.
    /// </summary>
    ///</remarks>
    static HRESULT TakeOwnershipAndGrantFullControl(HRESULT (*callback)(GUID), GUID guid);

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

    /// <summary>
    /// Deletes the component category registry key for the specified drive GUID under
    /// "Component Categories\{00021493-0000-0000-C000-000000000046}\Implementations".
    /// This is used to remove the shell folder registration for a drive.
    /// </summary>
    static HRESULT DeleteComponentCategoryRegistryKey(GUID guid);

    /// <summary>
    /// Creates the component category registry key for the specified drive GUID under
    /// "Component Categories\{00021493-0000-0000-C000-000000000046}\Implementations".
    /// This is used to register the shell folder for a drive in the component category.
    /// </summary>
    static HRESULT CreateComponentCategoryRegistryKey(GUID guidDrive);

    /// <summary>
    /// Retrieves the current process's user SID and returns it in the output parameter.
    /// The caller is responsible for freeing the returned SID with free().
    /// </summary>
    static HRESULT GetCurrentProcessSID(PSID* pOwner);

    /// <summary>
    /// Enables the specified privilege in the current process token, such as SeTakeOwnershipPrivilege.
    /// This is required for certain registry and security operations.
    /// </summary>
    static HRESULT EnablePrivilege(LPCWSTR privilege);

};

