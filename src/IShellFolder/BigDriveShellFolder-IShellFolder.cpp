// <copyright file="BigDriveShellFolder-IShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"
#include "EmptyEnumIDList.h"

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
    // Validate output pointer
    if (!ppidl) 
    {
        if (pchEaten) *pchEaten = 0;
        return E_INVALIDARG;
    }

    *ppidl = nullptr;

    // Validate input
    if (!pszDisplayName || !*pszDisplayName) 
    {
        if (pchEaten) *pchEaten = 0;
        return E_INVALIDARG;
    }

    // For a minimal implementation, just create a simple one-level PIDL for the display name
    size_t len = wcslen(pszDisplayName);
    if (pchEaten) *pchEaten = static_cast<ULONG>(len);

    // Allocate a minimal PIDL: [cb][data...][cb=0]
    // We'll use the display name as the "data" for the PIDL
    size_t nameLen = (len + 1) * sizeof(wchar_t);
    size_t pidlSize = sizeof(USHORT) + nameLen + sizeof(USHORT); // [cb][name][cb=0]
    BYTE* pidlMem = (BYTE*)::CoTaskMemAlloc(pidlSize);
    if (!pidlMem) 
    {
        return E_OUTOFMEMORY;
    }

    // Fill in the PIDL
    USHORT* pcb = (USHORT*)pidlMem;
    *pcb = static_cast<USHORT>(nameLen); // size of this item
    memcpy(pidlMem + sizeof(USHORT), pszDisplayName, nameLen);
    USHORT* pcbEnd = (USHORT*)(pidlMem + sizeof(USHORT) + nameLen);
    *pcbEnd = 0; // null terminator for the PIDL

    *ppidl = (PIDLIST_RELATIVE)pidlMem;

    // Optionally set attributes
    if (pdwAttributes) 
    {
        *pdwAttributes = SFGAO_FILESYSTEM | SFGAO_FOLDER;
    }

    return S_OK;
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
    // Validate output pointer
    if (!ppenumIDList)
    {
        return E_INVALIDARG;
    }

    *ppenumIDList = nullptr;

    // For a minimal implementation, return an empty enumerator (no files/folders)
    // This is sufficient for a shell folder that is empty or as a stub for a drive root.

    *ppenumIDList = new EmptyEnumIDList();
    return S_OK;
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
    if (!pidl || !ppv)
    {
        return E_INVALIDARG;
    }

    *ppv = nullptr; 

    PIDLIST_ABSOLUTE pidlSubFolder = ILCombine(m_pidl, pidl);

    BigDriveShellFolder* pSubFolder = new (std::nothrow) BigDriveShellFolder(m_driveGuid, this, pidlSubFolder);
    if (!pSubFolder)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pSubFolder->QueryInterface(riid, ppv);
    if (FAILED(hr))
    {
        goto End;
    }

End:

    if (pSubFolder!= nullptr)
    {
        pSubFolder->Release(); 
        pSubFolder = nullptr;
    }

    return hr;
}

/// <summary>
/// Binds to the storage of a specified object in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Compares two item IDs to determine their relative order.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Creates a view object for the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::CreateViewObject(HWND hwndOwner, REFIID riid, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Retrieves the attributes of one or more items in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut)
{
    if (cidl == 0 || !apidl || !rgfInOut)
    {
        return E_INVALIDARG;
    }

    *rgfInOut = SFGAO_FILESYSTEM;

    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Retrieves an object that can be used to carry out actions on the specified items.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
    REFIID riid, UINT* rgfReserved, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Retrieves the display name of an item in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Sets the display name of an item in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName,
    SHGDNF uFlags, PITEMID_CHILD* ppidlOut)
{
    // Placeholder implementation
    return E_NOTIMPL;
}
