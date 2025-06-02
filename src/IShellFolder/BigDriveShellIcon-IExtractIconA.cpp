// <copyright file="BigDriveShellIcon-IExtractIconA.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellIcon.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <commctrl.h>
#include <commoncontrols.h>

// Need this to link properly
#pragma comment(lib, "comctl32.lib")

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
HRESULT __stdcall BigDriveShellIcon::GetIconLocation(
	UINT uFlags,
	LPSTR pszFile,
	UINT cchMax,
	int* pIndex,
	UINT* pwFlags)
{
	HRESULT hr = S_OK;
	const BIGDRIVE_ITEMID* pItem = nullptr;

	if (!pszFile || !pIndex || !pwFlags)
	{
		return E_INVALIDARG;
	}

	// Initialize flags
	*pwFlags = 0;

	switch (m_cidl)
	{
	case 0:
		// No items selected -- Use default icon
		return S_FALSE;

	case 1:

		// Single item selected
		pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(m_apidl[0]);
		if (!pItem)
		{
			return S_FALSE;
		}

		switch (static_cast<BigDriveItemType>(pItem->uType))
		{
		case BigDriveItemType_Folder:

			// For folders, use standard folder icon
			hr = StringCchCopyA(pszFile, cchMax, "shell32.dll");
			if (SUCCEEDED(hr))
			{
				*pIndex = 3;  // Standard folder icon
			}
			break;

		case BigDriveItemType_File:
		{
			// For files, get the extension and ask the shell for the icon
			char szExt[MAX_PATH] = ".";

			// Extract extension from the name
			LPCWSTR pszName = pItem->szName;
			LPCWSTR pszExtension = wcsrchr(pszName, L'.');

			if (!pszExtension)
			{
				// No extension - use generic document icon
				StringCchCopyA(pszFile, cchMax, "shell32.dll");
				*pIndex = 1;

				goto End;
			}

			// Convert the wide extension to ANSI for use with ANSI APIs
			char szAnsiExt[MAX_PATH] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, pszExtension, -1, szAnsiExt, MAX_PATH, NULL, NULL);

			// Create dummy filename with extension
			char szDummyFile[MAX_PATH] = "dummy";
			strcat_s(szDummyFile, MAX_PATH, szAnsiExt);

			// Query shell for icon
			SHFILEINFOA shfi = { 0 };
			DWORD_PTR result = SHGetFileInfoA(
				szDummyFile,
				FILE_ATTRIBUTE_NORMAL,
				&shfi,
				sizeof(shfi),
				SHGFI_ICONLOCATION | SHGFI_USEFILEATTRIBUTES
			);

			if (!result)
			{
				// Fallback to generic document icon
				::StringCchCopyA(pszFile, cchMax, "shell32.dll");
				*pIndex = 1;

				goto End;
			}

			if (shfi.szDisplayName[0] != '\0')
			{
				// Normal case - icon location is in a specific file
				::StringCchCopyA(pszFile, cchMax, shfi.szDisplayName);
				*pIndex = shfi.iIcon;
			}
			else if (shfi.iIcon != 0)
			{
				// Special case - icon is in the system image list
				// Use shell32.dll which is the main shell icon resource
				::StringCchCopyA(pszFile, cchMax, "shell32.dll");

				// GIL_NOTFILENAME is set, the shell will calls the Extract() method
				*pwFlags |= GIL_NOTFILENAME;  // Tell shell this isn't a real path
				*pIndex = shfi.iIcon;  // Pass the system image list index
			}
			else
			{
				// Fallback - no icon found
				::StringCchCopyA(pszFile, cchMax, "shell32.dll");
				*pIndex = 1;  // Generic document icon
			}

			break;
		}
		default:
			// Unknown type - use default icon
			return S_FALSE;
		}
		break;

	default:
		// Multiple items selected
		return S_FALSE;  // Use default icon
	}

End:

	return hr;
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
HRESULT __stdcall BigDriveShellIcon::Extract(
	LPCSTR pszFile,
	UINT nIconIndex,
	HICON* phiconLarge,
	HICON* phiconSmall,
	UINT nIconSize)
{
	// Check if we're dealing with a system image list icon
	// (when using shell32.dll with GIL_NOTFILENAME flag)
	if (strcmp(pszFile, "shell32.dll") == 0 && nIconIndex > 0)
	{
		// Get the system image lists
		IImageList* pImageListLarge = nullptr;
		IImageList* pImageListSmall = nullptr;
		HRESULT hr = S_OK;

		// Get large icon image list (32x32)
		if (phiconLarge)
		{
			hr = SHGetImageList(SHIL_LARGE, IID_IImageList, (void**)&pImageListLarge);
			if (SUCCEEDED(hr) && pImageListLarge)
			{
				// Extract icon from the system image list
				hr = pImageListLarge->GetIcon(nIconIndex, ILD_TRANSPARENT, phiconLarge);
				if (FAILED(hr))
				{
					*phiconLarge = nullptr;
				}

				// Release the image list
				pImageListLarge->Release();
			}
			else
			{
				*phiconLarge = nullptr;
			}
		}

		// Get small icon image list (16x16)
		if (phiconSmall)
		{
			hr = SHGetImageList(SHIL_SMALL, IID_IImageList, (void**)&pImageListSmall);
			if (SUCCEEDED(hr) && pImageListSmall)
			{
				// Extract icon from the system image list
				hr = pImageListSmall->GetIcon(nIconIndex, ILD_TRANSPARENT, phiconSmall);
				if (FAILED(hr))
				{
					*phiconSmall = nullptr;
				}

				// Release the image list
				pImageListSmall->Release();
			}
			else
			{
				*phiconSmall = nullptr;
			}
		}

		// If we were able to extract at least one icon, return success
		if ((phiconLarge && *phiconLarge) || (phiconSmall && *phiconSmall))
		{
			return S_OK;
		}
	}

	// For standard icons or if system image list extraction failed,
	// let the Shell extract the icon using standard methods
	return S_FALSE;
}