// <copyright file="ETWManifestManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <strsafe.h>

#include "ETWManifestManager.h"
#include "LaunchDebugger.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

HRESULT ETWManifestManager::GetManifestPath(LPWSTR* ppManifestPath)
{
    HRESULT hr = S_OK;
    WCHAR modulePath[MAX_PATH] = { 0 };
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
    DWORD pathLength = GetModuleFileNameW((HINSTANCE)&__ImageBase, modulePath, MAX_PATH);
    if (pathLength == 0 || pathLength == MAX_PATH)
    {
        // Failed to get module path or buffer too small
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto End;
    }

    // Find the last backslash to extract the directory
    lastBackslash = wcsrchr(modulePath, L'\\');
    if (lastBackslash == NULL)
    {
        hr = E_UNEXPECTED;
        goto End;
    }

    // Calculate length needed for the full manifest path
    // Add 1 for null terminator and +19 for "BigDriveEvents.man"
    manifestPathLength = pathLength - (lastBackslash + 1 - modulePath) + 19;

    // Allocate memory for the manifest path using CoTaskMemAlloc
    manifestPath = (LPWSTR)::CoTaskMemAlloc(manifestPathLength * sizeof(WCHAR));
    if (manifestPath == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Copy directory part
    dirPartLength = lastBackslash + 1 - modulePath;
    hr = StringCchCopyNW(manifestPath, manifestPathLength, modulePath, dirPartLength);
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

HRESULT ETWManifestManager::BuildCommandLine(LPCWSTR action, LPCWSTR manifestPath, LPWSTR buffer, SIZE_T bufferSize)
{
    // Format the command line: wevtutil.exe [action] "[manifestPath]"
    HRESULT hr = StringCchPrintfW(buffer, bufferSize, L"wevtutil.exe %s \"%s\"", action, manifestPath);
    return hr;
}

HRESULT ETWManifestManager::RegisterManifest()
{
    HRESULT hr  = S_OK;
    LPWSTR manifestPath = nullptr;

     hr = GetManifestPath(&manifestPath);
    if (FAILED(hr))
    {
        return hr;
    }
    // Register the manifest
    hr = RegisterManifest(manifestPath);
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

HRESULT ETWManifestManager::RegisterManifest(LPCWSTR manifestPath)
{
    ::LaunchDebugger();

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