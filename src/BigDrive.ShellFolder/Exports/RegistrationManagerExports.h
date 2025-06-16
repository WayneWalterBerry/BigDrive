// <copyright file="RegistrationManagerExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\RegistrationManager.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) HRESULT RegisterShellFoldersFromRegistry();
    __declspec(dllexport) HRESULT GetRegisteredCLSIDs(CLSID** ppClsids, DWORD* pdwSize);
    __declspec(dllexport) HRESULT RegisterShellFolder(GUID guidDrive, BSTR bstrName);
    __declspec(dllexport) HRESULT CheckDllAndOSBitnessMatch(BOOL* pIsMatch);
    __declspec(dllexport) HRESULT CleanUpShellFolders();

#ifdef __cplusplus
}
#endif
