// <copyright file="dllmain.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

/// <summary>
/// Declares an external global variable representing the current instance handle of the application.
/// </summary>
extern HINSTANCE g_hInstance; 

/// <summary>
/// Entry point for the DLL. Handles process attach and detach events.
/// </summary>
/// <param name="hModule">Handle to the DLL module.</param>
/// <param name="ul_reason_for_call">Reason for the function call (e.g., process attach or detach).</param>
/// <param name="lpReserved">Reserved for future use.</param>
/// <returns>TRUE if successful; FALSE otherwise.</returns>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

/// <summary>
/// Registers the DLL by adding necessary registry entries.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" HRESULT __stdcall DllRegisterServer();

/// <summary>
/// Unregisters the DLL by removing registry entries.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" HRESULT __stdcall DllUnregisterServer();

/// <summary>
/// Retrieves a class factory object that can create instances of a COM class identified by the specified CLSID.
/// This function is called by COM clients to obtain an IClassFactory interface pointer for a registered COM class,
/// enabling the creation of COM objects (such as IShellFolder implementations) provided by this DLL.
/// 
/// The function first validates the output pointer, then enumerates all CLSIDs registered by the component.
/// If the requested CLSID matches one of the registered CLSIDs, it creates a new BigDriveShellFolderFactory instance
/// associated with that CLSID. It then queries the factory for the requested interface (typically IClassFactory)
/// and returns the interface pointer to the caller. If the CLSID is not found or an error occurs, the function
/// returns CLASS_E_CLASSNOTAVAILABLE or the appropriate HRESULT error code.
/// 
/// All allocated resources are released before returning. This function is essential for COM activation and
/// integration with Windows Explorer shell extensions.
/// </summary>
/// <param name="rclsid">The CLSID of the COM class object to retrieve.</param>
/// <param name="riid">The interface identifier (IID) of the interface to retrieve (usually IID_IClassFactory).</param>
/// <param name="ppv">Address of pointer variable that receives the interface pointer requested in riid.</param>
/// <returns>S_OK on success, CLASS_E_CLASSNOTAVAILABLE if the CLSID is not supported, or another HRESULT error code.</returns>
extern "C" STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv);

/// <summary>
/// Registers the BigDrive context menu extension for "My PC" (This PC) in the Windows Shell.
/// This function performs all required registry modifications to enable the extension:
/// <list type="number">
///   <item>
///     <description>Creates or updates the registry key under HKCR\CLSID\{CLSID_BigDriveExtension} to register the COM object, including a friendly name.</description>
///   </item>
///   <item>
///     <description>Creates or updates the InprocServer32 subkey for the CLSID, setting the default value to the full path of the extension DLL and the ThreadingModel to "Apartment".</description>
///   </item>
///   <item>
///     <description>Registers the extension under HKCR\CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\shellex\ContextMenuHandlers\BigDriveExtension, associating it with "My Computer" (This PC) so the shell will load the extension when displaying its context menu.</description>
///   </item>
/// </list>
/// All registry operations are performed with error checking and proper cleanup. If any step fails, the function returns an appropriate HRESULT error code.
/// </summary>
/// <param name="clsidExtension">The CLSID of the context menu extension to register.</param>
/// <returns>HRESULT indicating success (S_OK) or failure (error code).</returns>
HRESULT RegisterContextMenuExtension(const CLSID& clsidExtension);

HRESULT GetModuleFileNameW(LPWSTR szModulePath, DWORD dwSize);
HRESULT IsCurrentOS64Bit(bool& is64Bit);
HRESULT IsDll64Bit(const wchar_t* dllPath, bool& is64Bit);
HRESULT CheckDllAndOSBitnessMatch(bool& isMatch);
