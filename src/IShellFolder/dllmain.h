// <copyright file="dllmain.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <objbase.h>

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
extern "C" __declspec(dllexport) HRESULT __stdcall DllRegisterServer();

/// <summary>
/// Unregisters the DLL by removing registry entries.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" __declspec(dllexport) HRESULT __stdcall DllUnregisterServer();

/// <summary>
/// Retrieves a class object from the DLL for the specified CLSID.
/// </summary>
/// <param name="rclsid">The CLSID of the object to retrieve.</param>
/// <param name="riid">The interface identifier (IID) for the requested interface.</param>
/// <param name="ppv">Pointer to the location where the interface pointer will be stored.</param>
/// <returns>HRESULT indicating success or failure.</returns>
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv);

/// <summary>
/// Exportable Version For Testing
/// Retrieves a class object from the DLL for the specified CLSID.
/// </summary>
/// <param name="rclsid">The CLSID of the object to retrieve.</param>
/// <param name="riid">The interface identifier (IID) for the requested interface.</param>
/// <param name="ppv">Pointer to the location where the interface pointer will be stored.</param>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" __declspec(dllexport) HRESULT __stdcall DllGetClassObjectExport(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv);
