// <copyright file="dllmain.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <debugapi.h>
#include <objbase.h>
#include <sstream>
#include <windows.h>

#include "BigDriveShellFolderFactory.h"
#include "CLSIDs.h"
#include "LaunchDebugger.h"
#include "RegistrationManager.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{

    HRESULT hr = S_OK;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize COM
        hr = CoInitialize(NULL);
        if (FAILED(hr)) {
            // Handle the error (e.g., log it or return FALSE to indicate failure)
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        // Uninitialize COM
        CoUninitialize();
        break;
    }

    return TRUE;
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllRegisterServer()
{
    ::LaunchDebugger();

    return RegistrationManager::GetInstance().RegisterShellFoldersFromRegistry();
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllUnregisterServer()
{
    // Unregister from My Computer namespace - CHANGED TO USE HKEY_CURRENT_USER
    RegDeleteKeyW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{D4E5F6A7-B8C9-0123-4567-89ABCDEF1234}");

    // Unregister CLSID_BigDriveShellFolder  
    if (RegDeleteTreeW(HKEY_CLASSES_ROOT, L"CLSID\\D4E5F6A7-B8C9-0123-4567-89ABCDEF1234") == ERROR_SUCCESS) {
        return S_OK;
    }
    return E_FAIL;
}

/// <summary>
/// Retrieves a class object from a DLL for the specified CLSID.
/// This function is typically implemented in a DLL that provides COM objects,
/// allowing clients to obtain an IShellFolder instance.
/// 
/// Usage:
/// Clients call DllGetClassObject() to retrieve an IClassFactory for IShellFolder,
/// which is then used to instantiate the requested shell folder object.
/// </summary>
/// <param name="rclsid">The CLSID of the object to retrieve.</param>
/// <param name="riid">The interface identifier (IID) for the requested interface.</param>
/// <param name="ppv">Pointer to the location where the interface pointer will be stored.</param>
/// <returns>HRESULT indicating success or failure.</returns>
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
    HRESULT hrReturn = S_OK;

    ::LaunchDebugger();

    if (ppv == nullptr) 
    {
        return E_POINTER;
    }

    // Ensure the output pointer is initialized to nullptr.
    *ppv = nullptr;

    CLSID* pclsid = nullptr;
    BigDriveShellFolderFactory* pFactory = nullptr;

    hrReturn = RegistrationManager::GetInstance().GetRegisteredCLSIDs(&pclsid);
    if (FAILED(hrReturn))
    {
        return hrReturn;
    }

    for (int i = 0; pclsid[i] != GUID_NULL; i++)
    {
        if (pclsid[i] == rclsid)
        {
            // The CLSID matches, create the factory, with the CLSID as the GUID
            pFactory = new BigDriveShellFolderFactory(rclsid);
            if (!pFactory) 
            {
                return E_OUTOFMEMORY;
            }

            hrReturn = pFactory->QueryInterface(riid, ppv);
            if (FAILED(hrReturn))
            {
                goto End;
            }

            goto End;
        }
    }

    hrReturn = CLASS_E_CLASSNOTAVAILABLE;

End:

    // Clean Up

    if (pFactory)
    {
        pFactory->Release();
        pFactory = nullptr;
    }

    if (pclsid)
    {
        CoTaskMemFree(pclsid);
        pclsid = nullptr;
    }

    return hrReturn;
}


