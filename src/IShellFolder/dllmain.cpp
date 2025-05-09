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
    // ::LaunchDebugger();

    // return RegistrationManager::GetInstance().RegisterShellFoldersFromRegistry();

    return S_OK;
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

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
    if (ppv == nullptr) {
        return E_POINTER;
    }

    *ppv = nullptr; // Ensure the output pointer is initialized to nullptr.

    if (rclsid == CLSID_BigDriveShellFolder) {
        BigDriveShellFolderFactory* pFactory = new (std::nothrow) BigDriveShellFolderFactory();
        if (!pFactory) {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = pFactory->QueryInterface(riid, ppv);
        pFactory->Release(); // Release the initial reference held by the factory
        return hr;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}


