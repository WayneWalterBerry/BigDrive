// <copyright file="BigDriveShellFolder-IShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"
#include "EmptyEnumIDList.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "BigDriveEnumIDList.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"
#include "..\BigDrive.Client\DriveConfiguration.h"
#include "BigDriveShellIcon.h"
#include "ILExtensions.h"

/// <summary>
/// Parses a display name and returns a PIDL (Pointer to an Item ID List) that uniquely identifies an item
/// within the BigDrive shell namespace extension, which emulates a hard drive under "This PC".
///
/// <para><b>Parameters:</b></para>
/// <param name="hwnd">
///   [in] Handle to the owner window for any UI that may be displayed. Typically unused; may be NULL.
/// </param>
/// <param name="pbc">
///   [in, optional] Pointer to a bind context used during parsing. Can be used to pass parameters or objects
///   between the caller and the parsing function. May be NULL if not needed.
/// </param>
/// <param name="pszDisplayName">
///   [in] The Unicode string containing the display name to parse. This is typically a path or item name
///   relative to the root of the namespace (e.g., "Folder1\\File.txt").
/// </param>
/// <param name="pchEaten">
///   [out, optional] Pointer to a ULONG that receives the number of characters of pszDisplayName that were
///   successfully parsed. Set to the number of characters consumed if parsing is successful; otherwise, set to 0.
/// </param>
/// <param name="ppidl">
///   [out] Address of a PIDLIST_RELATIVE pointer that receives the PIDL corresponding to the parsed item.
///   On success, this must be allocated with CoTaskMemAlloc and must be freed by the caller with CoTaskMemFree.
///   Set to nullptr on failure.
/// </param>
/// <param name="pdwAttributes">
///   [in, out, optional] Pointer to a ULONG specifying the attributes to query. On output, receives the
///   attributes of the parsed item (e.g., SFGAO_FOLDER, SFGAO_FILESYSTEM). May be NULL if not needed.
/// </param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the display name was successfully parsed and a PIDL was returned.
///   Returns a COM error code (e.g., E_INVALIDARG, E_OUTOFMEMORY, E_FAIL) if parsing fails.
/// </returns>
///
/// <para><b>Behavior and Notes:</b></para>
/// <list type="bullet">
///   <item>pszDisplayName should be parsed relative to the root of the BigDrive namespace. For example,
///         "Folder1\\File.txt" should resolve to the corresponding item within the virtual drive.</item>
///   <item>If the item does not exist or cannot be resolved, return a failure HRESULT and set *ppidl to nullptr.</item>
///   <item>If pchEaten is not NULL, set it to the number of characters successfully parsed from pszDisplayName.</item>
///   <item>If pdwAttributes is not NULL, set the output value to the attributes of the resolved item (using SFGAO_* flags).</item>
///   <item>The returned PIDL must be allocated with CoTaskMemAlloc and must be released by the caller using CoTaskMemFree.</item>
///   <item>Do not display UI unless absolutely necessary; this method is typically called by the shell for programmatic parsing.</item>
/// </list>
///
/// <para><b>Typical Usage:</b></para>
/// <list type="bullet">
///   <item>Used by the shell to convert a user-typed path or drag-and-drop target into a PIDL for navigation or binding.</item>
///   <item>Enables support for features like "Go To" in Explorer, command-line navigation, and programmatic access to items.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::ParseDisplayName(
	HWND hwnd,
	LPBC pbc,
	LPOLESTR pszDisplayName,
	ULONG* pchEaten,
	PIDLIST_RELATIVE* ppidl,
	ULONG* pdwAttributes)
{
	HRESULT hr = S_OK;

	m_traceLogger.LogParseDisplayName(__FUNCTION__, pszDisplayName);

	// Validate output pointer
	if (!ppidl)
	{
		if (pchEaten) *pchEaten = 0;
		hr = E_INVALIDARG;
		goto End;
	}

	*ppidl = nullptr;

	// Validate input
	if (!pszDisplayName || !*pszDisplayName)
	{
		if (pchEaten) *pchEaten = 0;
		hr = E_INVALIDARG;
		goto End;
	}

	// Use AllocBigDrivePidl to create a valid PIDL
	// For demonstration, default to folder type. You may want to parse the name to determine type.
	hr = AllocBigDrivePidl(BigDriveItemType_Folder, pszDisplayName, *ppidl);
	if (FAILED(hr) || !*ppidl)
	{
		if (pchEaten)
		{
			*pchEaten = 0;
		}

		goto End;
	}

	if (pchEaten)
	{
		*pchEaten = static_cast<ULONG>(wcslen(pszDisplayName));
	}

	if (pdwAttributes)
	{
		*pdwAttributes = SFGAO_FILESYSTEM | SFGAO_FOLDER;
	}

End:

	if (FAILED(hr))
	{
		if (ppidl && *ppidl)
		{
			::CoTaskMemFree(*ppidl);
			*ppidl = nullptr;
		}
		if (pchEaten) *pchEaten = 0;
	}

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Enumerates the objects (files and folders) contained within the BigDrive shell folder, which emulates a hard drive
/// under "This PC" in Windows Explorer.
///
/// <para><b>Parameters:</b></para>
/// <param name="hwnd">
///   [in] Handle to the owner window for any UI that may be displayed. Typically unused; may be NULL.
/// </param>
/// <param name="grfFlags">
///   [in] Flags that specify which items to include in the enumeration. This is a combination of SHCONTF_* values, such as:
///   <list type="bullet">
///     <item>SHCONTF_FOLDERS - Include folders in the enumeration.</item>
///     <item>SHCONTF_NONFOLDERS - Include non-folder items (files).</item>
///     <item>SHCONTF_INCLUDEHIDDEN - Include hidden items.</item>
///   </list>
///   The implementation should honor these flags to filter the results appropriately.
/// </param>
/// <param name="ppenumIDList">
///   [out] Address of a pointer to an IEnumIDList interface. On success, receives the enumerator object that can be used
///   to iterate the child items (as PIDLs) in the folder. The caller is responsible for releasing this interface.
///   Set to nullptr on failure.
/// </param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the enumerator was successfully created and returned in *ppenumIDList.
///   Returns a COM error code (e.g., E_INVALIDARG, E_OUTOFMEMORY, E_FAIL) if enumeration fails.
/// </returns>
///
/// <para><b>Behavior and Notes:</b></para>
/// <list type="bullet">
///   <item>The enumerator should return a PIDL for each file or folder in the BigDrive namespace, filtered according to grfFlags.</item>
///   <item>If the folder is empty or no items match the filter, the enumerator should return S_FALSE from its Next() method.</item>
///   <item>If ppenumIDList is NULL, return E_INVALIDARG.</item>
///   <item>The returned IEnumIDList must be implemented to support enumeration of the current folder's children.</item>
///   <item>Do not display UI unless absolutely necessary; this method is typically called by the shell for programmatic enumeration.</item>
/// </list>
///
/// <para><b>Typical Usage:</b></para>
/// <list type="bullet">
///   <item>Used by the shell to populate the contents of the folder view in Explorer.</item>
///   <item>Enables enumeration for drag-and-drop, context menus, and other shell operations.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList** ppenumIDList)
{
	HRESULT hr = S_OK;
	DriveConfiguration driveConfiguration;
	BigDriveInterfaceProvider* pInterfaceProvider = nullptr;
	BSTR folderName = nullptr;
	LONG lowerBound = 0, upperBound = 0;
	IBigDriveEnumerate* pBigDriveEnumerate = nullptr;
	SAFEARRAY* psafolders = nullptr;
	SAFEARRAY* psaFiles = nullptr;
	BSTR bstrPath = nullptr;
	BSTR bstrFolderName = nullptr;
	BSTR bstrFileName = nullptr;
	LPITEMIDLIST pidl = nullptr;
	BigDriveEnumIDList* pResult = nullptr;
	LONG lCount = 0;

	m_traceLogger.LogEnter(__FUNCTION__);

	// Validate output pointer
	if (!ppenumIDList)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	*ppenumIDList = nullptr;

	hr = BigDriveConfigurationClient::GetDriveConfiguration(m_driveGuid, driveConfiguration);
	if (FAILED(hr))
	{
		WriteErrorFormatted(L"EnumObjects: Failed to get drive configuration. HRESULT: 0x%08X", hr);
		goto End;
	}

	pInterfaceProvider = new BigDriveInterfaceProvider(driveConfiguration);
	if (pInterfaceProvider == nullptr)
	{
		WriteError(L"EnumObjects: Failed to create BigDriveInterfaceProvider");
		hr = E_OUTOFMEMORY;
		goto End;
	}

	hr = pInterfaceProvider->GetIBigDriveEnumerate(&pBigDriveEnumerate);
	switch (hr)
	{
	case S_OK:
		break;
	case S_FALSE:
		// Interface isn't Implemented By The Provider
		goto End;
	default:
		WriteErrorFormatted(L"EnumObjects: Failed to obtain IBigDriveEnumerate, HRESULT: 0x%08X", hr);
		break;
	}

	if (pBigDriveEnumerate == nullptr)
	{
		hr = E_FAIL;
		WriteErrorFormatted(L"EnumObjects: Failed to obtain IBigDriveEnumerate, HRESULT: 0x%08X", hr);
		goto End;
	}

	hr = GetPathForProviders(m_pidlAbsolute, bstrPath);
	if (FAILED(hr))
	{
		goto End;
	}

	/// Folders and Files are enumerated separately, so we need to check the flags
	if (grfFlags & SHCONTF_FOLDERS)
	{
		hr = pBigDriveEnumerate->EnumerateFolders(m_driveGuid, bstrPath, &psafolders);
		if (FAILED(hr) || (psafolders == nullptr))
		{
			goto End;
		}

		::SafeArrayGetLBound(psafolders, 1, &lowerBound);
		::SafeArrayGetUBound(psafolders, 1, &upperBound);

		lCount = upperBound - lowerBound + 1;

		if (pResult == nullptr)
		{
			pResult = new BigDriveEnumIDList(lCount);
			if (!pResult)
			{
				hr = E_OUTOFMEMORY;
				goto End;
			}
		}

		for (LONG i = lowerBound; i <= upperBound; ++i)
		{
			::SafeArrayGetElement(psafolders, &i, &bstrFolderName);

			// Allocate the Relative PIDL to pass back to 
			hr = AllocBigDrivePidl(BigDriveItemType_Folder, bstrFolderName, pidl);
			if (FAILED(hr) || (pidl == nullptr))
			{
				goto End;
			}

			if (pidl == nullptr)
			{
				hr = E_FAIL;
				goto End;
			}

			hr = pResult->Add(pidl);
			if (FAILED(hr))
			{
				goto End;
			}

			if (pidl)
			{
				::CoTaskMemFree(pidl);
				pidl = nullptr;
			}

			if (bstrFolderName)
			{
				::SysFreeString(bstrFolderName);
				bstrFolderName = nullptr;
			}
		}
	}

	if (grfFlags & SHCONTF_NONFOLDERS)
	{
		hr = pBigDriveEnumerate->EnumerateFiles(m_driveGuid, bstrPath, &psaFiles);
		if (FAILED(hr) || (psaFiles == nullptr))
		{
			goto End;
		}

		::SafeArrayGetLBound(psaFiles, 1, &lowerBound);
		::SafeArrayGetUBound(psaFiles, 1, &upperBound);

		lCount = upperBound - lowerBound + 1;

		if (pResult == nullptr)
		{
			pResult = new BigDriveEnumIDList(lCount);
			if (!pResult)
			{
				hr = E_OUTOFMEMORY;
				goto End;
			}
		}

		for (LONG i = lowerBound; i <= upperBound; ++i)
		{
			::SafeArrayGetElement(psaFiles, &i, &bstrFileName);

			// Allocate the Relative PIDL to pass back to 
			hr = AllocBigDrivePidl(BigDriveItemType_File, bstrFileName, pidl);
			if (FAILED(hr) || (pidl == nullptr))
			{
				goto End;
			}

			if (pidl == nullptr)
			{
				hr = E_FAIL;
				goto End;
			}

			hr = pResult->Add(pidl);
			if (FAILED(hr))
			{
				goto End;
			}

			if (pidl)
			{
				::CoTaskMemFree(pidl);
				pidl = nullptr;
			}

			if (bstrFileName)
			{
				::SysFreeString(bstrFileName);
				bstrFileName = nullptr;
			}
		}
	}

	if (pResult == nullptr)
	{
		*ppenumIDList = new EmptyEnumIDList();
	}
	else
	{
		*ppenumIDList = pResult;
	}

	m_traceLogger.LogResults(__FUNCTION__, *ppenumIDList);

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	if (FAILED(hr) && pResult)
	{
		delete pResult;
		pResult = nullptr;
	}

	if (psafolders)
	{
		::SafeArrayDestroy(psafolders);
		psafolders = nullptr;
	}

	if (psaFiles)
	{
		::SafeArrayDestroy(psaFiles);
		psaFiles = nullptr;
	}

	if (pidl)
	{
		::CoTaskMemFree(pidl);
		pidl = nullptr;
	}

	if (pBigDriveEnumerate)
	{
		pBigDriveEnumerate->Release();
		pBigDriveEnumerate = nullptr;
	}

	if (bstrFolderName)
	{
		::SysFreeString(bstrFolderName);
		bstrFolderName = nullptr;
	}

	if (bstrFileName)
	{
		::SysFreeString(bstrFileName);
		bstrFileName = nullptr;
	}

	if (bstrPath)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	if (pInterfaceProvider)
	{
		delete pInterfaceProvider;
		pInterfaceProvider = nullptr;
	}

	return hr;
}

/// <summary>
/// Binds to a subfolder or item within the BigDrive shell namespace extension, which emulates a hard drive under "This PC".
/// This method is called by the shell when it needs to obtain an interface pointer (such as IShellFolder) for a child item
/// represented by a PIDL (Pointer to an Item ID List).
///
/// <para><b>Parameters:</b></para>
/// <param name="pidl">
///   [in] The PIDL that identifies the child object to bind to. This is typically a relative PIDL representing a subfolder
///   or item within the current folder. The PIDL must be parsed and validated by the implementation.
/// </param>
/// <param name="pbc">
///   [in, optional] Pointer to a bind context used during the binding operation. Can be used to pass parameters or objects
///   between the caller and the binding function. May be NULL if not needed.
/// </param>
/// <param name="riid">
///   [in] The interface identifier (IID) of the interface the caller is requesting (e.g., IID_IShellFolder).
/// </param>
/// <param name="ppv">
///   [out] Address of a pointer that receives the requested interface pointer for the bound object. On success, this
///   will point to the requested interface (such as a new IShellFolder for a subfolder). The caller is responsible for
///   releasing this interface. Set to nullptr on failure.
/// </param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the binding was successful and the requested interface was returned in *ppv.
///   Returns E_INVALIDARG if any required parameter is invalid, E_NOINTERFACE if the requested interface is not supported,
///   or another COM error code if binding fails.
/// </returns>
///
/// <para><b>Behavior and Notes:</b></para>
/// <list type="bullet">
///   <item>The method should parse the input PIDL to identify the target child object (subfolder or item) within the drive namespace.</item>
///   <item>If the requested interface is IShellFolder and the PIDL represents a subfolder, return a new IShellFolder instance for that subfolder.</item>
///   <item>If the PIDL does not correspond to a valid child object, or the requested interface is not supported, return E_NOINTERFACE and set *ppv to nullptr.</item>
///   <item>The returned interface pointer must be properly reference-counted and released by the caller.</item>
///   <item>Do not display UI unless absolutely necessary; this method is typically called by the shell for programmatic binding.</item>
/// </list>
///
/// <para><b>Typical Usage:</b></para>
/// <list type="bullet">
///   <item>Used by the shell to navigate into subfolders or items within the emulated drive, enabling folder tree expansion and navigation.</item>
///   <item>Enables the shell to obtain IShellFolder or other interfaces for child objects, supporting features like folder views, context menus, and drag-and-drop.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;
	PIDLIST_ABSOLUTE pidlSubFolder = nullptr;
	BigDriveShellFolder* pSubFolder = nullptr;

	m_traceLogger.LogEnter(__FUNCTION__, riid, m_pidlAbsolute, pidl);

	if (!pidl || !ppv)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	*ppv = nullptr;

	pidlSubFolder = ::ILCombine(m_pidlAbsolute, pidl);

	hr = BigDriveShellFolder::Create(m_driveGuid, this, pidlSubFolder, &pSubFolder);
	if (FAILED(hr))
	{
		goto End;
	}

	hr = pSubFolder->QueryInterface(riid, ppv);
	if (FAILED(hr))
	{
		goto End;
	}

End:

	if (pSubFolder != nullptr)
	{
		pSubFolder->Release();
		pSubFolder = nullptr;
	}

	if (pidlSubFolder != nullptr)
	{
		::ILFree(pidlSubFolder);
		pidlSubFolder = nullptr;
	}

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Binds to the storage of a specified object in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
	HRESULT hr = E_NOTIMPL;

	m_traceLogger.LogEnter(__FUNCTION__);

	// Placeholder implementation

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Compares two item IDs (PIDLs) to determine their relative order within the current shell folder.
/// This method is called by the Windows Shell to sort items in views (such as Explorer windows),
/// to group items, and to determine if two PIDLs refer to the same object.
/// 
/// <para><b>Parameters:</b></para>
/// <param name="lParam">
///   [in] A value that specifies the comparison criteria. For most shell extensions, this is 0,
///   but it may be a column index or sort flag for custom views.
/// </param>
/// <param name="pidl1">
///   [in] The first relative PIDL to compare. This PIDL is relative to the current folder.
/// </param>
/// <param name="pidl2">
///   [in] The second relative PIDL to compare. This PIDL is also relative to the current folder.
/// </param>
/// 
/// <para><b>Return Value:</b></para>
/// <returns>
///   Returns an HRESULT with the result in the low-order word:
///   - Negative value: pidl1 precedes pidl2
///   - Zero:           pidl1 and pidl2 are equivalent
///   - Positive value: pidl1 follows pidl2
///   The high-order word must be zero.
/// </returns>
/// 
/// <para><b>Behavior and Notes:</b></para>
/// <list type="bullet">
///   <item>For most extensions, comparison is based on the display name or a unique identifier in the PIDL.</item>
///   <item>If the PIDLs are equal (refer to the same item), return 0.</item>
///   <item>If sorting by name, use a case-insensitive comparison of the item names stored in the PIDLs.</item>
///   <item>For custom columns or sort orders, use lParam to select the comparison criteria.</item>
///   <item>Do not return E_NOTIMPL; the shell requires this method for sorting and grouping.</item>
///   <item>Only compare the child (last) SHITEMID in each PIDL, not the full chain.</item>
/// </list>
/// 
/// <para><b>Typical Usage:</b></para>
/// <list type="bullet">
///   <item>Used by the shell to sort items in folder views, group items, and check for equality.</item>
///   <item>Called frequently during enumeration and display of items in Explorer.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
	HRESULT hr = E_NOTIMPL;
	int cmpResult = 0;
	const BIGDRIVE_ITEMID* pItem1;
	const BIGDRIVE_ITEMID* pItem2;

	m_traceLogger.LogEnter(__FUNCTION__, pidl1, pidl2);

	if (!pidl1 || !pidl2)
	{
		s_eventLogger.WriteErrorFormmated(L"CompareIDs: Invalid PIDL pointers", E_INVALIDARG);
		hr = E_INVALIDARG;
		goto End;
	}

	if (!::ILIsEqual(pidl1, pidl2))
	{
		// If the PIDLs are equal, return 0
		hr = S_OK;
		goto End;
	}

	if (!IsValidBigDriveItemId(pidl1) || !IsValidBigDriveItemId(pidl2))
	{
		// Can only compare valid BigDrive item IDs
		hr = S_OK;
		goto End;
	}

	// Cast to BIGDRIVE_ITEMID
	pItem1 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pidl1);
	pItem2 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pidl2);

	if (!pItem1 || !pItem2 || !pItem1->szName || !pItem2->szName)
	{
		s_eventLogger.WriteErrorFormmated(L"Invalid PIDL or item name in CompareIDs");

		// Treat as equal if invalid
		return 0; 
	}

	// Compare szName (case-insensitive)
	cmpResult = ::lstrcmpiW(pItem1->szName, pItem2->szName);

	// Return as required by IShellFolder: negative, zero, or positive in LOWORD
	hr = HRESULT_FROM_WIN32((cmpResult < 0) ? -1 : (cmpResult > 0) ? 1 : 0);

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Creates a view object for the BigDrive shell folder, enabling the Windows Shell to display the contents
/// of the folder in a user interface such as Windows Explorer. This method is called by the shell when it
/// needs to create a view (e.g., a folder window, details pane, or other UI component) for the folder.
/// 
/// <para><b>Parameters:</b></para>
/// <param name="hwndOwner">
///   [in] Handle to the owner window for any UI that may be displayed. Typically used as the parent window
///   for any dialogs or UI elements created by the view object. May be NULL if not applicable.
/// </param>
/// <param name="riid">
///   [in] The interface identifier (IID) of the view object the shell is requesting. Commonly requested
///   interfaces include IID_IShellView (for folder views), IID_IDropTarget (for drag-and-drop support),
///   and others depending on the shell's needs.
/// </param>
/// <param name="ppv">
///   [out] Address of a pointer that receives the requested interface pointer for the view object. On
///   success, this will point to the requested interface. The caller is responsible for releasing this
///   interface. Set to nullptr on failure.
/// </param>
/// 
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the requested view object was successfully created and returned in *ppv.
///   E_NOINTERFACE if the requested interface is not supported by this folder.
///   E_INVALIDARG if any required parameter is invalid.
///   E_NOTIMPL if the method is not implemented (typical for minimal or stub shell extensions).
///   Other COM error codes as appropriate.
/// </returns>
/// 
/// <para><b>Behavior and Notes:</b></para>
/// <list type="bullet">
///   <item>The shell calls this method to obtain a UI object for displaying or interacting with the folder's contents.</item>
///   <item>If the requested interface is IID_IShellView, the implementation should return an object that implements the IShellView interface, which is responsible for rendering the folder's items in Explorer.</item>
///   <item>Other interfaces, such as IDropTarget or IContextMenu, may also be requested depending on shell operations.</item>
///   <item>If the requested interface is not supported, return E_NOINTERFACE and set *ppv to nullptr.</item>
///   <item>For minimal or stub implementations, it is common to return E_NOTIMPL to indicate that no view object is provided.</item>
///   <item>The returned interface pointer must be properly reference-counted and released by the caller.</item>
///   <item>Do not display UI unless absolutely necessary; this method is typically called by the shell for programmatic view creation.</item>
/// </list>
/// 
/// <para><b>Typical Usage:</b></para>
/// <list type="bullet">
///   <item>Used by the shell to create the folder view window when the user navigates into the folder in Explorer.</item>
///   <item>Enables support for drag-and-drop, context menus, and other UI features by returning the appropriate interfaces.</item>
///   <item>Required for shell extensions that want to provide a custom view or UI for their namespace.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::CreateViewObject(HWND hwndOwner, REFIID riid, void** ppv)
{
	HRESULT hr = E_NOINTERFACE;

	if (!ppv)
	{
		s_eventLogger.WriteErrorFormmated(L"CreateViewObject: Invalid Pointer", hr);
		return E_INVALIDARG;
	}

	*ppv = nullptr;

	m_traceLogger.LogEnter(__FUNCTION__, riid);

	if (IsEqualIID(riid, IID_IShellView))
	{
		SFV_CREATE sfv = {};
		sfv.cbSize = sizeof(sfv);
		sfv.pshf = static_cast<IShellFolder*>(this);
		sfv.psvOuter = nullptr;
		sfv.psfvcb = nullptr;

		// SHCreateShellFolderView() Requires that IShellView2::GetDetailsOf() be implemented, if it isn't implemented
		// then the shell will not be able to display the folder contents.
		hr = ::SHCreateShellFolderView(&sfv, reinterpret_cast<IShellView**>(ppv));
		if (FAILED(hr))
		{
			s_eventLogger.WriteErrorFormmated(L"CreateViewObject: Failed to Create IShellView. HRESULT: 0x%08X", hr);
			goto End;
		}
	}

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Retrieves the attributes of one or more items in the folder for the Windows Shell.
/// The Shell calls this method to determine the capabilities and characteristics of the specified items,
/// such as whether they are folders, files, support renaming, have subfolders, or are part of the file system.
/// The returned attributes (SFGAO_* flags) are used by the Shell to enable or disable UI features,
/// optimize operations, and control user interactions (e.g., context menus, drag-and-drop, property sheets).
///
/// <para><b>Shell Expectations:</b></para>
/// <list type="bullet">
///   <item>The method must fill in the output attribute flags for all specified items, using the SFGAO_* constants.</item>
///   <item>If multiple items are specified, the method should return the intersection of the attributes that apply to all items.</item>
///   <item>If any parameter is invalid or attributes cannot be determined, return an error and do not modify the output.</item>
///   <item>This method must not display UI or block for a long time; it is called frequently and must be efficient.</item>
/// </list>
///
/// <para><b>Parameters:</b></para>
/// <param name="cidl">
///   [in] The number of items for which attributes are requested. Must be greater than 0.
/// </param>
/// <param name="apidl">
///   [in] An array of pointers to item ID lists (PIDLs), each identifying an item relative to this folder.
/// </param>
/// <param name="rgfInOut">
///   [in, out] On input, a bitmask specifying which attributes the caller is interested in.
///   On output, receives the bitmask of SFGAO_* flags that apply to all specified items.
/// </param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the attributes were successfully retrieved and set in <paramref name="rgfInOut"/>.
///   E_INVALIDARG if any parameter is invalid (e.g., cidl == 0, apidl == nullptr, or rgfInOut == nullptr).
///   Other COM error codes as appropriate.
/// </returns>
///
/// <para><b>Output:</b></para>
/// <list type="bullet">
///   <item><paramref name="rgfInOut"/> is set to a bitmask of SFGAO_* flags describing the items' attributes.</item>
///   <item>Common flags: SFGAO_FOLDER, SFGAO_FILESYSTEM, SFGAO_HASSUBFOLDER, SFGAO_FILESYSANCESTOR, SFGAO_STORAGE.</item>
/// </list>
///
/// <para><b>Notes:</b></para>
/// <list type="bullet">
///   <item>Do not allocate or free memory for the PIDLs; ownership remains with the caller.</item>
///   <item>Return only the attributes that apply to all items if more than one is specified.</item>
///   <item>This method is called frequently by the Shell and must be fast and reliable.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut)
{
	HRESULT hr = S_OK;

	// Start with all bits set for intersection
	SFGAOF resultFlags = ~0ULL; 

	m_traceLogger.LogEnter(__FUNCTION__, cidl, apidl);

	if (cidl == 0 || !apidl || !rgfInOut)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	for (UINT i = 0; i < cidl; ++i)
	{
		const BIGDRIVE_ITEMID* pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(apidl[i]);
		SFGAOF itemFlags = 0;

		if (!pItem)
		{
			hr = E_INVALIDARG;
			goto End;
		}

		switch (static_cast<BigDriveItemType>(pItem->uType))
		{
		case BigDriveItemType_Folder:
			itemFlags = SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_FILESYSANCESTOR | SFGAO_BROWSABLE;
			break;
		case BigDriveItemType_File:
			itemFlags = SFGAO_FILESYSTEM | SFGAO_STREAM;
			break;
		default:
			itemFlags = 0;
			break;
		}

		resultFlags &= itemFlags;
	}

	*rgfInOut = resultFlags;

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);
	return hr;
}

/// <summary>
/// Retrieves a COM interface object that enables actions or UI operations on one or more items in the folder.
/// The Windows Shell calls this method to obtain interfaces such as IContextMenu, IDataObject, IDropTarget,
/// or IShellIcon for the specified items, enabling features like context menus, drag-and-drop, clipboard operations,
/// and custom icons in Explorer.
/// 
/// <para><b>Shell Expectations:</b></para>
/// <list type="bullet">
///   <item>The method must validate all input parameters and return E_INVALIDARG if any are invalid.</item>
///   <item>The method should support standard interfaces requested by the Shell, such as IID_IContextMenu,
///         IID_IDataObject, IID_IDropTarget, and IID_IShellIcon, as appropriate for the items.</item>
///   <item>If the requested interface is not supported, return E_NOINTERFACE and set *ppv to nullptr.</item>
///   <item>The returned interface pointer must be properly reference-counted; the Shell will release it when done.</item>
///   <item>Do not display UI or block for a long time; this method is called frequently and must be efficient.</item>
/// </list>
/// 
/// <para><b>Parameters:</b></para>
/// <param name="hwndOwner">
///   [in] Handle to the owner window for any UI that may be displayed. Typically used for context menu or property sheet dialogs.
///   May be NULL if not needed.
/// </param>
/// <param name="cidl">
///   [in] The number of items for which the interface is requested. If 1, apidl[0] is the single item; if >1, applies to all.
/// </param>
/// <param name="apidl">
///   [in] Array of pointers to item ID lists (PIDLs), each identifying an item relative to this folder.
/// </param>
/// <param name="riid">
///   [in] The interface identifier (IID) of the requested interface (e.g., IID_IContextMenu, IID_IDataObject).
/// </param>
/// <param name="rgfReserved">
///   [in] Reserved. Must be set to NULL by the caller.
/// </param>
/// <param name="ppv">
///   [out] Address of a pointer that receives the requested interface pointer on success. Set to nullptr on failure.
/// </param>
/// 
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the requested interface was successfully created and returned in *ppv.<br/>
///   E_NOINTERFACE if the requested interface is not supported.<br/>
///   E_INVALIDARG if any required parameter is invalid.<br/>
///   Other COM error codes as appropriate.
/// </returns>
/// 
/// <para><b>Output:</b></para>
/// <list type="bullet">
///   <item>*ppv receives the requested interface pointer if successful; otherwise, set to nullptr.</item>
/// </list>
/// 
/// <para><b>Notes:</b></para>
/// <list type="bullet">
///   <item>This method is called by the Shell for context menus, drag-and-drop, clipboard, and icon operations.</item>
///   <item>Implement only the interfaces relevant to your namespace extension; return E_NOINTERFACE for others.</item>
///   <item>Do not allocate or free memory for the PIDLs; ownership remains with the caller.</item>
///   <item>Do not display UI unless required for the requested interface (e.g., context menu invocation).</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, UINT* rgfReserved, void** ppv)
{
	HRESULT hr = E_NOTIMPL;
	BigDriveShellIcon* pBigDriveShellIcon = nullptr;

	m_traceLogger.LogEnter(__FUNCTION__, riid);

	if (!ppv || !apidl || cidl == 0 || (cidl > 1 && !rgfReserved))
	{
		hr = E_INVALIDARG;
		goto End;
	}

	*ppv = nullptr;

	if ((riid == IID_IExtractIconW) || (riid == IID_IExtractIconA))
	{
		pBigDriveShellIcon = new BigDriveShellIcon(m_driveGuid, this, cidl, apidl);
		if (!pBigDriveShellIcon)
		{
			hr = E_OUTOFMEMORY;
			goto End;
		}

		hr = pBigDriveShellIcon->QueryInterface(riid, ppv);
		if (FAILED(hr))
		{
			this->Release();
			goto End;
		}
	}
	else
	{
		hr = E_NOINTERFACE;
		goto End;
	}

End:

	if (pBigDriveShellIcon != nullptr)
	{
		pBigDriveShellIcon->Release();
		pBigDriveShellIcon = nullptr;
	}

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Retrieves the display name of an item in the BigDrive shell folder namespace.
/// This method is called by the Windows Shell when it needs to obtain a user-friendly or parsing name for an item,
/// such as when displaying the item in Explorer, showing tooltips, or generating paths for drag-and-drop, copy, or property dialogs.
///
/// <para><b>How the Shell Calls This Method:</b></para>
/// The shell calls GetDisplayNameOf() whenever it needs to display or use the name of an item represented by a PIDL.
/// This includes populating folder views, address bars, tooltips, context menus, and when resolving paths for shell operations.
/// The shell may call this method multiple times for the same item with different flags to obtain different name formats.
///
/// <para><b>Parameters:</b></para>
/// <param name="pidl">
///   [in] The relative PIDL (Pointer to an Item ID List) that identifies the item within this folder.
///   This PIDL is typically created by ParseDisplayName or returned by EnumObjects.
/// </param>
/// <param name="uFlags">
///   [in] Flags (of type SHGDNF) that specify the type of display name to retrieve. Common values include:
///   <list type="bullet">
///     <item>SHGDN_NORMAL - The default display name for UI (e.g., file/folder name).</item>
///     <item>SHGDN_FORPARSING - A name suitable for parsing (e.g., a full path or canonical name).</item>
///     <item>SHGDN_INFOLDER - The name relative to the parent folder.</item>
///   </list>
///   These flags may be combined to request specific formats.
/// </param>
/// <param name="pName">
///   [out] Pointer to a STRRET structure that receives the display name. The method must fill this structure
///   with the requested name in one of the supported formats (OLE string, C string, or offset).
///   The caller is responsible for freeing any allocated memory as indicated by the STRRET type.
/// </param>
///
/// <para><b>Return Value:</b></para>
/// <returns>
///   S_OK if the display name was successfully retrieved and returned in pName.
///   Returns a COM error code (e.g., E_INVALIDARG, E_FAIL) if the operation fails.
/// </returns>
///
/// <para><b>Behavior and Notes:</b></para>
/// <list type="bullet">
///   <item>The method should extract the item's name from the PIDL and format it according to the requested flags.</item>
///   <item>For SHGDN_NORMAL, return the user-visible name (e.g., "File.txt" or "Folder").</item>
///   <item>For SHGDN_FORPARSING, return a fully qualified name or path suitable for parsing (e.g., "C:\BigDrive\File.txt").</item>
///   <item>For SHGDN_INFOLDER, return the name relative to the parent folder.</item>
///   <item>The returned name must be placed in the STRRET structure using the appropriate type (STRRET_WSTR, STRRET_CSTR, or STRRET_OFFSET).</item>
///   <item>If the PIDL is invalid or the name cannot be determined, return an error code and do not modify pName.</item>
///   <item>Do not display UI; this method is for programmatic retrieval of names only.</item>
/// </list>
///
/// <para><b>Typical Usage:</b></para>
/// <list type="bullet">
///   <item>Used by the shell to display item names in folder views, address bars, and dialogs.</item>
///   <item>Used to generate parsing names for drag-and-drop, copy, and property operations.</item>
///   <item>Called frequently during enumeration, navigation, and shell operations.</item>
/// </list>
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName)
{
	HRESULT hr = E_NOTIMPL;

	m_traceLogger.LogEnter(__FUNCTION__);

	if (!pidl || !pName)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	if (::ILGetCount(pidl) != 1)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	if ((uFlags & SHGDN_FORPARSING) == SHGDN_FORPARSING)
	{
		hr = GetBigDriveItemNameFromPidl(pidl, pName);
		if (FAILED(hr))
		{
			goto End;
		}
	}
	else
	{
		hr = GetBigDriveItemNameFromPidl(pidl, pName);
		if (FAILED(hr))
		{
			goto End;
		}
	}

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <summary>
/// Sets the display name of an item in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut)
{
	HRESULT hr = E_NOTIMPL;

	m_traceLogger.LogEnter(__FUNCTION__);

	// Placeholder implementation

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}
