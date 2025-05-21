// <copyright file="RegistrationManagerExports.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <oaidl.h> 

#include "RegistrationManagerExports.h"

extern "C" {

    HRESULT RegisterShellFoldersFromRegistry() 
    {
        return RegistrationManager::RegisterShellFoldersFromRegistry();
    }

    HRESULT GetRegisteredCLSIDs(CLSID** ppClsids, DWORD* pdwSize) 
    {
        if (!pdwSize) return E_POINTER;
        DWORD dwSize = 0;
        HRESULT hr = RegistrationManager::GetRegisteredCLSIDs(ppClsids, dwSize);
        *pdwSize = dwSize;
        return hr;
    }

    HRESULT RegisterShellFolder(GUID guidDrive, BSTR bstrName) 
    {
        return RegistrationManager::RegisterShellFolder(guidDrive, bstrName);
    }

    HRESULT CheckDllAndOSBitnessMatch(BOOL* pIsMatch) 
    {
        if (!pIsMatch) return E_POINTER;
        bool isMatch = false;
        HRESULT hr = RegistrationManager::CheckDllAndOSBitnessMatch(isMatch);
        *pIsMatch = isMatch ? TRUE : FALSE;
        return hr;
    }

    HRESULT CleanUpShellFolders() 
    {
        return RegistrationManager::CleanUpShellFolders();
    }

} // extern "C"
