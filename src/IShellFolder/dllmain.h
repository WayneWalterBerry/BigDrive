// <copyright file="dllmain.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#ifdef BIGDRIVE_SHELLFOLDER_EXPORTS
#define BIGDRIVE_API __declspec(dllexport)
#else
#define BIGDRIVE_API __declspec(dllimport)
#endif

#include <windows.h>

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
extern "C" BIGDRIVE_API HRESULT __stdcall DllRegisterServer();

/// <summary>
/// Unregisters the DLL by removing registry entries.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" BIGDRIVE_API HRESULT __stdcall DllUnregisterServer();

extern "C" HRESULT __stdcall DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv);

