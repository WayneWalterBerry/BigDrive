// <copyright file="BigDriveShellFolder-IExtractIconA.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveShellFolder.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

/// <summary>
/// Retrieves the location and index of the icon for a specified item in the BigDrive shell namespace (ANSI version).
/// The Shell calls this method to determine which icon to display for a given item (file or folder).
///
/// <para><b>Parameters:</b></para>
/// <param name="uFlags">[in] Flags specifying icon retrieval options (GIL_*).</param>
/// <param name="pszFile">[out] Buffer to receive the icon location (DLL or EXE path, or special string, ANSI).</param>
/// <param name="cchMax">[in] Size of the pszFile buffer, in characters.</param>
/// <param name="pIndex">[out] Receives the icon index within the file specified by pszFile.</param>
/// <param name="pwFlags">[in, out] On input, specifies icon retrieval flags (GIL_*). On output, can specify additional flags.</param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the icon location and index were successfully retrieved.<br/>
///   S_FALSE if a default icon should be used.<br/>
///   E_FAIL or other COM error codes on failure.
/// </returns>
///
/// <para><b>Notes:</b></para>
/// <list type="bullet">
///   <item>Set *pszFile to the path of the icon file (e.g., system DLL or EXE).</item>
///   <item>Set *pIndex to the icon index within the file.</item>
///   <item>Set *pwFlags to GIL_PERINSTANCE if the icon is per-instance, or GIL_NOTFILENAME if pszFile is not a file path.</item>
///   <item>Return S_FALSE to let the Shell use the default icon.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetIconLocation(
    UINT uFlags,
    LPSTR pszFile,
    UINT cchMax,
    int* pIndex,
    UINT* pwFlags)
{
    // For demonstration, always return the standard folder icon from shell32.dll
    if (!pszFile || !pIndex || !pwFlags)
        return E_INVALIDARG;

    // Use the system's shell32.dll as the icon source (ANSI)
    HRESULT hr = StringCchCopyA(pszFile, cchMax, "shell32.dll");
    if (FAILED(hr))
        return hr;

    // Use icon index 3 for folders, 1 for files (standard indices in shell32.dll)
    *pIndex = 3; // Folder icon in shell32.dll
    *pwFlags = GIL_PERINSTANCE;

    return S_OK;
}

/// <summary>
/// Extracts the icon image for a specified item in the BigDrive shell namespace (ANSI version).
/// The Shell calls this method if GetIconLocation returns S_OK and expects the actual icon handle.
///
/// <para><b>Parameters:</b></para>
/// <param name="pszFile">[in] The icon location string returned by GetIconLocation (ANSI).</param>
/// <param name="nIconIndex">[in] The icon index within the file.</param>
/// <param name="phiconLarge">[out] Receives the large icon handle (32x32).</param>
/// <param name="phiconSmall">[out] Receives the small icon handle (16x16).</param>
/// <param name="nIconSize">[in] Specifies the desired icon sizes (LOWORD = large, HIWORD = small).</param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the icon(s) were successfully extracted.<br/>
///   S_FALSE if the Shell should extract the icon itself.<br/>
///   E_FAIL or other COM error codes on failure.
/// </returns>
///
/// <para><b>Notes:</b></para>
/// <list type="bullet">
///   <item>If you do not provide the icon, return S_FALSE to let the Shell extract it using the location and index.</item>
///   <item>If you return icon handles, the Shell will destroy them when done.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::Extract(
    LPCSTR pszFile,
    UINT nIconIndex,
    HICON* phiconLarge,
    HICON* phiconSmall,
    UINT nIconSize)
{
    // For most extensions, return S_FALSE to let the Shell extract the icon using the location and index
    // If you want to provide your own icon, load it and return the handles

    UNREFERENCED_PARAMETER(pszFile);
    UNREFERENCED_PARAMETER(nIconIndex);
    UNREFERENCED_PARAMETER(phiconLarge);
    UNREFERENCED_PARAMETER(phiconSmall);
    UNREFERENCED_PARAMETER(nIconSize);

    return S_FALSE;
}