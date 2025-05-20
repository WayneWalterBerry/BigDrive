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
    if (!ok)
    {
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

HRESULT TrustedInstaller::EnablePrivilege(LPCWSTR privName)
{
    HANDLE hToken = nullptr;
    TOKEN_PRIVILEGES tp = {};
    LUID luid = {};

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return HRESULT_FROM_WIN32(GetLastError());

    if (!LookupPrivilegeValueW(NULL, privName, &luid)) {
        CloseHandle(hToken);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
        CloseHandle(hToken);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // AdjustTokenPrivileges may succeed, but GetLastError() could still indicate failure
    DWORD lastErr = GetLastError();
    CloseHandle(hToken);
    return (lastErr == ERROR_SUCCESS) ? S_OK : HRESULT_FROM_WIN32(lastErr);
}

HRESULT TrustedInstaller::CheckSeDebugPrivilege()
{
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        printf("OpenProcessToken failed: %lu\n", GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // First, get the required buffer size
    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenPrivileges, nullptr, 0, &dwSize);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        printf("GetTokenInformation (size query) failed: %lu\n", GetLastError());
        CloseHandle(hToken);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Allocate buffer and get the privileges
    PTOKEN_PRIVILEGES pPrivs = (PTOKEN_PRIVILEGES)malloc(dwSize);
    if (!pPrivs) {
        printf("Memory allocation failed\n");
        CloseHandle(hToken);
        return E_OUTOFMEMORY;
    }

    if (!GetTokenInformation(hToken, TokenPrivileges, pPrivs, dwSize, &dwSize)) {
        printf("GetTokenInformation failed: %lu\n", GetLastError());
        free(pPrivs);
        CloseHandle(hToken);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Lookup the LUID for SE_DEBUG_NAME
    LUID luid;
    if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) {
        printf("LookupPrivilegeValue failed: %lu\n", GetLastError());
        free(pPrivs);
        CloseHandle(hToken);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Check if SE_DEBUG_NAME is enabled
    BOOL found = FALSE;
    BOOL enabled = FALSE;
    for (DWORD i = 0; i < pPrivs->PrivilegeCount; ++i) {
        if (pPrivs->Privileges[i].Luid.LowPart == luid.LowPart &&
            pPrivs->Privileges[i].Luid.HighPart == luid.HighPart) {
            found = TRUE;
            if (pPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) {
                printf("SE_DEBUG_NAME is ENABLED\n");
                enabled = TRUE;
            }
            else {
                printf("SE_DEBUG_NAME is present but NOT enabled\n");
            }
            break;
        }
    }
    if (!found) {
        printf("SE_DEBUG_NAME is not present in the token\n");
    }

    free(pPrivs);
    CloseHandle(hToken);

    if (!found)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    if (!enabled)
        return HRESULT_FROM_WIN32(ERROR_PRIVILEGE_NOT_HELD);
    return S_OK;
}

HRESULT TrustedInstaller::Impersonate(HANDLE* hImpersonationToken)
{
    if (!hImpersonationToken) return E_POINTER;
    *hImpersonationToken = nullptr;

    DWORD tiPid = 0;
    HRESULT hr = StartService(&tiPid);
    if (FAILED(hr)) 
    {
        printf("Failed to start TrustedInstaller service.\n");
        return hr;
    }

	hr = EnablePrivilege(SE_DEBUG_NAME);
    if (FAILED(hr))
    {
        printf("Failed to enable SE_DEBUG_NAME privilege.\n");
        return hr;
	}

	hr = CheckSeDebugPrivilege();
    if (FAILED(hr))
    {
        printf("SE_DEBUG_NAME privilege is not enabled.\n");
		return hr;
    }

    HANDLE hProc = nullptr;
    hr = GetProcessHandle(tiPid, &hProc);
    if (FAILED(hr) || hProc == nullptr)
    {
        printf("Failed to open TrustedInstaller process. hr=0x%08X, hProc=%p\n", hr, hProc);
        return hr;
    }

    HANDLE hToken = nullptr;
    if (!::OpenProcessToken(hProc, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) 
    {
        DWORD err = GetLastError();
        printf("Failed to open TrustedInstaller process token.\n");
        ::CloseHandle(hProc);
        return HRESULT_FROM_WIN32(err);
    }

    HANDLE hDupToken = nullptr;
    if (!::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenImpersonation, &hDupToken))
    {
        DWORD err = GetLastError();
        printf("Failed to duplicate token.\n");
        ::CloseHandle(hToken);
        ::CloseHandle(hProc);
        return HRESULT_FROM_WIN32(err);
    }

    if (!::ImpersonateLoggedOnUser(hDupToken)) 
    {
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
