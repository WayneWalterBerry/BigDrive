// <copyright file="RegistrationManagerExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) HRESULT __stdcall CleanUpShellFoldersExport();
    __declspec(dllimport) HRESULT __stdcall RegisterShellFolderExport(GUID guidDrive, BSTR bstrName);
    __declspec(dllimport) HRESULT __stdcall GetModuleFileNameWExport(LPWSTR szModulePath, DWORD dwSize);

#ifdef __cplusplus
}
#endif