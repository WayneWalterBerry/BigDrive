// <copyright file="TrustedInstaller.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "TrustedInstaller.h"

#include <tlhelp32.h>
#include <stdio.h>
#include <string>
#include <comdef.h>

HRESULT TrustedInstaller::StartService(DWORD* tiPid)
{
    if (!tiPid) return E_POINTER;
    *tiPid = 0;

    SC_HANDLE hSCM = ::OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SC_HANDLE hSvc = ::OpenServiceW(hSCM, L"TrustedInstaller", SERVICE_QUERY_STATUS | SERVICE_START);
    if (!hSvc)
    {
        ::CloseServiceHandle(hSCM);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SERVICE_STATUS_PROCESS ssp = {};
    DWORD bytesNeeded = 0;
    BOOL ok = ::QueryServiceStatusEx(hSvc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded);
    if (!ok) {
        DWORD err = GetLastError();
        ::CloseServiceHandle(hSvc);
        ::CloseServiceHandle(hSCM);
        return HRESULT_FROM_WIN32(err);
    }

    if (ssp.dwCurrentState != SERVICE_RUNNING)
    {
        if (!::StartServiceW(hSvc, 0, NULL))
        {
            DWORD err = GetLastError();
            if (err != ERROR_SERVICE_ALREADY_RUNNING)
            {
                ::CloseServiceHandle(hSvc);
                ::CloseServiceHandle(hSCM);
                return HRESULT_FROM_WIN32(err);
            }
        }

        // Wait for the service to start
        for (int i = 0; i < 50; ++i)
        {
            Sleep(100);

            ::QueryServiceStatusEx(hSvc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded);
            if (ssp.dwCurrentState == SERVICE_RUNNING)
            {
                break;
            }
        }
    }

    *tiPid = ssp.dwProcessId;
    ::CloseServiceHandle(hSvc);
    ::CloseServiceHandle(hSCM);

    return (ssp.dwCurrentState == SERVICE_RUNNING && *tiPid != 0) ? S_OK : E_FAIL;
}

HRESULT TrustedInstaller::GetProcessHandle(DWORD tiPid, HANDLE* hProcess)
{
    if (!hProcess) return E_POINTER;
    *hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, tiPid);
    return *hProcess ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT TrustedInstaller::Impersonate(HANDLE* hImpersonationToken)
{
    if (!hImpersonationToken) return E_POINTER;
    *hImpersonationToken = nullptr;

    DWORD tiPid = 0;
    HRESULT hr = StartService(&tiPid);
    if (FAILED(hr)) {
        printf("Failed to start TrustedInstaller service.\n");
        return hr;
    }

    HANDLE hProc = nullptr;
    hr = GetProcessHandle(tiPid, &hProc);
    if (FAILED(hr)) {
        printf("Failed to open TrustedInstaller process.\n");
        return hr;
    }

    HANDLE hToken = nullptr;
    if (!::OpenProcessToken(hProc, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) {
        DWORD err = GetLastError();
        printf("Failed to open TrustedInstaller process token.\n");
        ::CloseHandle(hProc);
        return HRESULT_FROM_WIN32(err);
    }

    HANDLE hDupToken = nullptr;
    if (!::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenImpersonation, &hDupToken)) {
        DWORD err = GetLastError();
        printf("Failed to duplicate token.\n");
        ::CloseHandle(hToken);
        ::CloseHandle(hProc);
        return HRESULT_FROM_WIN32(err);
    }

    if (!::ImpersonateLoggedOnUser(hDupToken)) {
        DWORD err = GetLastError();
        printf("Failed to impersonate TrustedInstaller.\n");
        ::CloseHandle(hDupToken);
        ::CloseHandle(hToken);
        ::CloseHandle(hProc);
        return HRESULT_FROM_WIN32(err);
    }

    // Success: Now running as TrustedInstaller
    printf("Impersonation as TrustedInstaller succeeded.\n");

    ::CloseHandle(hToken);
    ::CloseHandle(hProc);

    *hImpersonationToken = hDupToken;
    return S_OK;
}

HRESULT TrustedInstaller::Revert(HANDLE hImpersonationToken)
{
    HRESULT hr = S_OK;
    if (!::RevertToSelf()) {
        printf("Failed to revert impersonation (RevertToSelf failed).\n");
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else {
        printf("Successfully reverted impersonation.\n");
    }

    if (hImpersonationToken) {
        ::CloseHandle(hImpersonationToken);
    }
    return hr;
}
