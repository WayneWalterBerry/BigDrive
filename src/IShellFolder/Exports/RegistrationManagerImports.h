// <copyright file="RegistrationManagerImports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\RegistrationManager.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) HRESULT RegisterShellFoldersFromRegistry();
    __declspec(dllimport) HRESULT GetRegisteredCLSIDs(CLSID** ppClsids, DWORD* pdwSize);
    __declspec(dllimport) HRESULT RegisterShellFolder(GUID guidDrive, BSTR bstrName);
    __declspec(dllimport) HRESULT CheckDllAndOSBitnessMatch(BOOL* pIsMatch);
    __declspec(dllimport) HRESULT CleanUpShellFolders();

#ifdef __cplusplus
}
#endif
