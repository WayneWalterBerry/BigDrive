// <copyright file="ETWManifestManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <strsafe.h>
#include <iostream>
#include <aclapi.h>
#include <sddl.h>

#include "ETWManifestManager.h"
#include "LaunchDebugger.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

HRESULT ETWManifestManager::GetManifestPath(LPWSTR* ppManifestPath)
{
    HRESULT hr = S_OK;
    WCHAR szModulePath[MAX_PATH] = { 0 };
    LPWSTR manifestPath = NULL;
    SIZE_T manifestPathLength = 0;
    SIZE_T dirPartLength = 0;
    DWORD fileAttributes;
    LPWSTR lastBackslash;

    if (ppManifestPath == NULL)
    {
        return E_POINTER;
    }

    // Initialize output parameter
    *ppManifestPath = NULL;

    // Get the path of the current DLL using __ImageBase
    DWORD pathLength = GetModuleFileNameW((HINSTANCE)&__ImageBase, szModulePath, MAX_PATH);
    if (pathLength == 0 || pathLength == MAX_PATH)
    {
        // Failed to get module path or buffer too small
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto End;
    }

    // Find the last backslash to extract the directory
    lastBackslash = wcsrchr(szModulePath, L'\\');
    if (lastBackslash == NULL)
    {
        hr = E_UNEXPECTED;
        goto End;
    }

    // Calculate length needed for the full manifest path
    // Add 1 for null terminator and +19 for "BigDriveEvents.man"
    manifestPathLength = pathLength - (lastBackslash + 1 - szModulePath) + 19;

    // Allocate memory for the manifest path using CoTaskMemAlloc
    manifestPath = (LPWSTR)::CoTaskMemAlloc(manifestPathLength * sizeof(WCHAR));
    if (manifestPath == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Copy directory part
    dirPartLength = lastBackslash + 1 - szModulePath;
    hr = StringCchCopyNW(manifestPath, manifestPathLength, szModulePath, dirPartLength);
    if (FAILED(hr))
    {
        goto End;
    }

    // Append filename
    hr = StringCchCatW(manifestPath, manifestPathLength, L"BigDriveEvents.man");
    if (FAILED(hr))
    {
        goto End;
    }

    // Verify the file exists
    fileAttributes = GetFileAttributesW(manifestPath);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto End;
    }

    // Success, set the output parameter
    *ppManifestPath = manifestPath;
    manifestPath = NULL; // Ownership transferred

End:
    // Clean up if there was an error
    if (manifestPath != NULL)
    {
        ::CoTaskMemFree(manifestPath);
    }

    return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::BuildCommandLine(LPCWSTR action, LPCWSTR manifestPath, LPWSTR buffer, SIZE_T bufferSize)
{
    // Format the command line: wevtutil.exe [action] "[manifestPath]"
    HRESULT hr = StringCchPrintfW(buffer, bufferSize, L"wevtutil.exe %s \"%s\"", action, manifestPath);
    return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::GrantEventLogServiceAccess()
{
    HRESULT hr = S_OK;
    WCHAR szModulePath[MAX_PATH] = { 0 };

    // Get the path of the current DLL using __ImageBase
    DWORD pathLength = GetModuleFileNameW((HINSTANCE)&__ImageBase, szModulePath, MAX_PATH);
    if (pathLength == 0 || pathLength == MAX_PATH)
    {
        // Failed to get module path or buffer too small
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto End;
    }

    bool bEventLogAccess;
    hr = CheckEventLogAccess(szModulePath, bEventLogAccess);
    if (FAILED(hr))
    {
        hr = E_ACCESSDENIED;
        goto End;
    }

    if (!bEventLogAccess)
    {
        // Grant access to the EventLog service
        hr = GrantEventLogAccess(szModulePath);
        if (FAILED(hr))
        {
            goto End;
        }
    }

End:

    return  hr;

}

/// <inheritdoc />
HRESULT ETWManifestManager::RegisterManifest()
{
    HRESULT hr = S_OK;
    LPWSTR szManifestPath = nullptr;

    hr = GetManifestPath(&szManifestPath);
    if (FAILED(hr))
    {
        return hr;
    }

    // The Event Log service (NT SERVICE\EventLog) needs read & execute access to the resource dll
    // to read the manifest and use the embedded resources.
    hr = GrantEventLogServiceAccess();
    if (FAILED(hr))
    {
        // Failed to grant access
        return hr;
    }

    // Register the manifest
    hr = RegisterManifest(szManifestPath);
    if (FAILED(hr))
    {
        // Failed to register the manifest
        goto End;
    }

    /*
    hr = VerifyManifestRegistration(szManifestPath);
    if (FAILED(hr))
    {
        // Failed to register the manifest
        goto End;
    }
    */

End:

    if (szManifestPath != nullptr)
    {
        // Free the manifest path memory
        ::CoTaskMemFree(szManifestPath);
        szManifestPath = nullptr;
    }

    return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::RegisterManifest(LPCWSTR manifestPath)
{
    // Maximum command line length (adjust if needed)
    const SIZE_T CMD_BUFFER_SIZE = 2048;
    WCHAR cmdLine[CMD_BUFFER_SIZE];

    // Build the command line for installation
    HRESULT hr = BuildCommandLine(L"im", manifestPath, cmdLine, CMD_BUFFER_SIZE);
    if (FAILED(hr))
    {
        // Failed to build command line (path too long)
        return hr;
    }

    return ExecuteWevtutil(cmdLine);
}

/// <inheritdoc />
HRESULT ETWManifestManager::UnregisterManifest()
{
    HRESULT hr = S_OK;
    LPWSTR manifestPath = nullptr;

    hr = GetManifestPath(&manifestPath);
    if (FAILED(hr))
    {
        return hr;
    }

    // Register the manifest
    hr = UnregisterManifest(manifestPath);
    if (FAILED(hr))
    {
        // Failed to register the manifest
        goto End;
    }

End:

    if (manifestPath != nullptr)
    {
        // Free the manifest path memory
        ::CoTaskMemFree(manifestPath);
        manifestPath = nullptr;
    }

    return hr;
}

HRESULT ETWManifestManager::UnregisterManifest(LPCWSTR manifestPath)
{
    // Maximum command line length (adjust if needed)
    const SIZE_T CMD_BUFFER_SIZE = 2048;
    WCHAR cmdLine[CMD_BUFFER_SIZE];

    // Build the command line for uninstallation
    HRESULT hr = BuildCommandLine(L"um", manifestPath, cmdLine, CMD_BUFFER_SIZE);
    if (FAILED(hr))
    {
        // Failed to build command line (path too long)
        return hr;
    }

    return ExecuteWevtutil(cmdLine);
}

HRESULT ETWManifestManager::ExecuteWevtutil(LPCWSTR cmdLine)
{
    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi = {};

    // Create a mutable copy of the command line
    // CreateProcessW requires a non-const LPWSTR
    SIZE_T bufferSize = lstrlenW(cmdLine) + 1;
    LPWSTR mutableCmdLine = new WCHAR[bufferSize];

    // Copy the string and ensure null termination
    HRESULT hr = StringCchCopyW(mutableCmdLine, bufferSize, cmdLine);
    if (FAILED(hr))
    {
        delete[] mutableCmdLine;
        return hr;
    }

    // Create process to execute wevtutil
    BOOL result = CreateProcessW(
        NULL,                   // No application name (use command line)
        mutableCmdLine,         // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,       // Don't create a window
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory
        &si,                    // Pointer to STARTUPINFO structure
        &pi                     // Pointer to PROCESS_INFORMATION structure
    );

    // Clean up the buffer as it's no longer needed
    delete[] mutableCmdLine;

    if (!result)
    {
        // Failed to create process
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Wait for the process to finish
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get the exit code
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    // Clean up process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Return success if process exited with exit code 0, otherwise return error
    return (exitCode == 0) ? S_OK : E_FAIL;
}

/// <inheritdoc />
HRESULT ETWManifestManager::CheckEventLogAccess(LPCWSTR path, bool& bHasAccess)
{
    if (path == NULL)
    {
        return E_POINTER;
    }

    bHasAccess = FALSE;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pACL = NULL;
    PSID pEventLogSID = NULL;
    HRESULT hr = S_OK;

    // Define file-specific access rights we require instead of generic rights
    // This is what GENERIC_READ | GENERIC_EXECUTE maps to for files
    ACCESS_MASK specificMask = FILE_GENERIC_READ | FILE_EXECUTE;

    // Get the security descriptor for the file
    DWORD result = ::GetNamedSecurityInfoW(path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pACL, NULL, &pSD);

    if (result != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(result);
        goto Cleanup;
    }

    hr = GetEventLogServiceSid(&pEventLogSID);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Check if the EventLog service has read/execute access

    for (WORD i = 0; i < pACL->AceCount; i++)
    {
        void* pACE;
        if (::GetAce(pACL, i, &pACE))
        {
            ACCESS_ALLOWED_ACE* ace = (ACCESS_ALLOWED_ACE*)pACE;

            LPWSTR lpszSid = NULL;
            ::ConvertSidToStringSidW((PSID)&ace->SidStart, &lpszSid);
            ::LocalFree(lpszSid);

            bool bEqualSid = ::EqualSid(pEventLogSID, (PSID)&ace->SidStart);

            // Check if all required access rights are included
            if (bEqualSid && ((ace->Mask & specificMask) == specificMask))
            {
                bHasAccess = TRUE;
                break;
            }
        }
    }

Cleanup:

    if (pSD != NULL)
    {
        ::LocalFree(pSD);
        pSD = NULL;
    }

    if (pEventLogSID != NULL)
    {
        ::LocalFree(pEventLogSID);
        pEventLogSID = NULL;
    }

    return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::GrantEventLogAccess(LPCWSTR path)
{
    HRESULT hr = S_OK;

    if (path == NULL)
    {
        return E_POINTER;
    }

    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pOldACL = NULL;
    PACL pNewACL = NULL;
    PSID pEventLogSID = NULL;
    EXPLICIT_ACCESS ea = { 0 };
    DWORD dwRes = 0;

    hr = GetEventLogServiceSid(&pEventLogSID);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Get the existing DACL
    dwRes = GetNamedSecurityInfoW(
        path,
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL,
        NULL,
        &pOldACL,
        NULL,
        &pSD);

    if (dwRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRes);
        goto Cleanup;
    }

    // Prepare the EXPLICIT_ACCESS structure for the new ACE
    ea.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPWSTR)pEventLogSID;

    // Create new ACL by merging the old one with our new entry
    dwRes = ::SetEntriesInAclW(1, &ea, pOldACL, &pNewACL);
    if (dwRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRes);
        goto Cleanup;
    }

    // Apply the new ACL to the file
    dwRes = ::SetNamedSecurityInfoW(
        (LPWSTR)path,
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL,
        NULL,
        pNewACL,
        NULL);

    if (dwRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRes);
        goto Cleanup;
    }

Cleanup:

    if (pSD != NULL)
    {
        LocalFree(pSD);
        pSD = NULL;
    }

    if (pNewACL != NULL)
    {
        LocalFree(pNewACL);
        pNewACL = NULL;
    }

    if (pEventLogSID != NULL)
    {
        LocalFree(pEventLogSID);
        pEventLogSID = NULL;
    }

    return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::VerifyManifestRegistration(LPCWSTR manifestPath)
{
    if (manifestPath == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    WCHAR regPath[MAX_PATH + 50] = L"SYSTEM\\CurrentControlSet\\Control\\WMI\\Manifests\\";
    WCHAR manifestName[MAX_PATH] = { 0 };
    const WCHAR* lastBackslash = wcsrchr(manifestPath, L'\\');

    // Extract the manifest filename without path
    if (lastBackslash != NULL)
    {
        hr = StringCchCopyW(manifestName, MAX_PATH, lastBackslash + 1);
    }
    else
    {
        hr = StringCchCopyW(manifestName, MAX_PATH, manifestPath);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    // Append manifest filename to registry path
    hr = StringCchCatW(regPath, MAX_PATH + 50, manifestName);
    if (FAILED(hr))
    {
        return hr;
    }

    // Open the registry key
    DWORD result = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        regPath,
        0,
        KEY_READ,
        &hKey);

    if (result != ERROR_SUCCESS)
    {
        // Registry key not found
        return HRESULT_FROM_WIN32(result);
    }

    // Check for required values
    DWORD valueType = 0;
    DWORD dataSize = 0;

    // Check for "ResourceFileName" value
    result = RegQueryValueExW(
        hKey,
        L"ResourceFileName",
        NULL,
        &valueType,
        NULL,
        &dataSize);

    if (result != ERROR_SUCCESS || valueType != REG_EXPAND_SZ)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(result != ERROR_SUCCESS ? result : ERROR_INVALID_DATA);
    }

    // Check for "BinaryPath" value
    result = RegQueryValueExW(
        hKey,
        L"BinaryPath",
        NULL,
        &valueType,
        NULL,
        &dataSize);

    if (result != ERROR_SUCCESS || valueType != REG_EXPAND_SZ)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(result != ERROR_SUCCESS ? result : ERROR_INVALID_DATA);
    }

    // Cleanup
    RegCloseKey(hKey);

    return S_OK;
}

/// <inheritdoc />
HRESULT ETWManifestManager::GetEventLogServiceSid(PSID *ppSid)
{
    HRESULT hr = S_OK;

    DWORD cbSid = 0;
    DWORD cchReferencedDomainName = 0;
    SID_NAME_USE eUse;
    WCHAR szDomainName[MAX_PATH] = {0};

    // First call to get required buffer sizes
    LookupAccountNameW(
        NULL,                       // local machine
        L"NT SERVICE\\EventLog",    // account name
        NULL,                       // Sid buffer
        &cbSid,                     // size of Sid buffer
        szDomainName,               // domain buffer
        &cchReferencedDomainName,   // size of domain buffer
        &eUse                       // SID_NAME_USE enum
    );

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Allocate required buffers
    *ppSid = (PSID)LocalAlloc(LPTR, cbSid);
    if (*ppSid == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Second call to get the actual SID
    if (!LookupAccountNameW(
        NULL,                      // local machine
        L"NT SERVICE\\EventLog",   // account name
        *ppSid,                      // Sid buffer
        &cbSid,                    // size of Sid buffer
        szDomainName,              // domain buffer
        &cchReferencedDomainName,  // size of domain buffer
        &eUse))                    // SID_NAME_USE enum
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LocalFree(*ppSid);
        *ppSid = NULL;
        return hr;
    }

    return S_OK;
}