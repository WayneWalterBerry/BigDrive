// <copyright file="BigDriveShellFolder.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <shlobj.h> // For IShellFolder and related interfaces
#include <objbase.h> // For COM initialization
#include <string>

// Local
#include "ILExtensions.h"

// Shared
#include "..\Shared\EventLogger.h"

/// <summary>
/// Represents a custom implementation of the IShellFolder interface for the BigDrive namespace.
/// Provides functionality for interacting with the shell folder hierarchy.
/// </summary>
class BigDriveShellFolder : public IShellFolder
{
private:

    /// <summary>
    /// The Drive guid 
    /// </summary>
    CLSID m_driveGuid;

    BigDriveShellFolder* m_pParentShellFolder;

    /// <summary>
    /// The relative PIDL (Pointer to an Item ID List) that identifies this shell folder's location 
    /// relative to its parent folder in the shell namespace. This PIDL is cloned and owned by the instance,
    /// and is released upon destruction.
    /// </summary>
    PCUIDLIST_RELATIVE m_pidl;

    /// <summary>
    /// Reference count for the COM object.
    /// </summary>
    LONG m_refCount;

public:

    /// <summary>
    /// Constructs a new instance of the BigDriveShellFolder class, representing a shell folder in the BigDrive namespace.
    /// Initializes the folder with the specified drive GUID and absolute PIDL, and sets the initial COM reference count to 1.
    /// The drive GUID uniquely identifies the virtual drive, while the PIDL specifies the folder's absolute location in the shell namespace.
    /// </summary>
    /// <param name="driveGuid">The GUID associated with the virtual drive or shell folder.</param>
    /// <param name="pParentShellFolder">Pointer to the parent shell folder, if any. Can be nullptr for root folders.</param>
    /// <param name="pidl">The absolute PIDL identifying the folder's location within the shell namespace.</param>
    BigDriveShellFolder(CLSID driveGuid, BigDriveShellFolder* pParentShellFolder, PCUIDLIST_RELATIVE pidl) :
        m_driveGuid(driveGuid), m_pParentShellFolder(pParentShellFolder), m_pidl(nullptr), m_refCount(1)
    {
        // Clone the PIDL to ensure it is owned by this instance
        m_pidl = ILClone(pidl);
    }

    ~BigDriveShellFolder()
    {
        // Free the PIDL when the object is destroyed
        if (m_pidl != nullptr)
        {
            ::ILFree(const_cast<LPITEMIDLIST>(m_pidl));
            m_pidl = nullptr;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IUnknown methods

    /// <summary>
    /// Queries the object for a pointer to one of its supported interfaces.
    /// </summary>
    /// <param name="riid">The identifier of the interface being requested.</param>
    /// <param name="ppvObject">A pointer to the interface pointer to be populated.</param>
    /// <returns>
    /// S_OK if the interface is supported; E_NOINTERFACE if not.
    /// </returns>
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

    /// <summary>
    /// Increments the reference count for the object.
    /// </summary>
    /// <returns>The new reference count.</returns>
    ULONG __stdcall AddRef() override;

    /// <summary>
    /// Decrements the reference count for the object. Deletes the object if the reference count reaches zero.
    /// </summary>
    /// <returns>The new reference count.</returns>
    ULONG __stdcall Release() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IShellFolder methods

    /// <summary>
    /// Parses a display name and returns a PIDL (Pointer to an Item ID List) that identifies the item.
    /// </summary>
    /// <param name="hwnd">A handle to the owner window for any UI that may be displayed.</param>
    /// <param name="pbc">A bind context that controls the parsing operation.</param>
    /// <param name="pszDisplayName">The display name to parse.</param>
    /// <param name="pchEaten">The number of characters of the display name that were parsed.</param>
    /// <param name="ppidl">A pointer to the PIDL that identifies the item.</param>
    /// <param name="pdwAttributes">Attributes of the item being parsed.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
        ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes) override;

    /// <summary>
    /// Enumerates the objects in the folder.
    /// </summary>
    /// <param name="hwnd">A handle to the owner window for any UI that may be displayed.</param>
    /// <param name="grfFlags">Flags that specify which items to include in the enumeration.</param>
    /// <param name="ppenumIDList">A pointer to the enumerator object.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList** ppenumIDList) override;

    /// <summary>
    /// Binds to a specified object in the folder.
    /// </summary>
    /// <param name="pidl">The PIDL of the object to bind to.</param>
    /// <param name="pbc">A bind context that controls the binding operation.</param>
    /// <param name="riid">The identifier of the interface to bind to.</param>
    /// <param name="ppv">A pointer to the interface pointer to be populated.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv) override;

    /// <summary>
    /// Binds to the storage of a specified object in the folder.
    /// </summary>
    /// <param name="pidl">The PIDL of the object to bind to.</param>
    /// <param name="pbc">A bind context that controls the binding operation.</param>
    /// <param name="riid">The identifier of the interface to bind to.</param>
    /// <param name="ppv">A pointer to the interface pointer to be populated.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv) override;

    /// <summary>
    /// Compares two item IDs to determine their relative order.
    /// </summary>
    /// <param name="lParam">A value that specifies how the comparison should be performed.</param>
    /// <param name="pidl1">The first item ID to compare.</param>
    /// <param name="pidl2">The second item ID to compare.</param>
    /// <returns>
    /// A negative value if pidl1 precedes pidl2; zero if they are equivalent; a positive value if pidl1 follows pidl2.
    /// </returns>
    HRESULT __stdcall CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2) override;

    /// <summary>
    /// Creates a view object for the folder.
    /// </summary>
    /// <param name="hwndOwner">A handle to the owner window for any UI that may be displayed.</param>
    /// <param name="riid">The identifier of the interface to create.</param>
    /// <param name="ppv">A pointer to the interface pointer to be populated.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall CreateViewObject(HWND hwndOwner, REFIID riid, void** ppv) override;

    /// <summary>
    /// Retrieves the attributes of one or more items in the folder.
    /// </summary>
    /// <param name="cidl">The number of items for which to retrieve attributes.</param>
    /// <param name="apidl">An array of item IDs.</param>
    /// <param name="rgfInOut">A pointer to a value that specifies and receives the attributes.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut) override;

    /// <summary>
    /// Retrieves an object that can be used to carry out actions on the specified items.
    /// </summary>
    /// <param name="hwndOwner">A handle to the owner window for any UI that may be displayed.</param>
    /// <param name="cidl">The number of items for which to retrieve the object.</param>
    /// <param name="apidl">An array of item IDs.</param>
    /// <param name="riid">The identifier of the interface to retrieve.</param>
    /// <param name="rgfReserved">Reserved. Must be NULL.</param>
    /// <param name="ppv">A pointer to the interface pointer to be populated.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall GetUIObjectOf(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
        REFIID riid, UINT* rgfReserved, void** ppv) override;

    /// <summary>
    /// Retrieves the display name of an item in the folder.
    /// </summary>
    /// <param name="pidl">The item ID of the item.</param>
    /// <param name="uFlags">Flags that specify the type of display name to retrieve.</param>
    /// <param name="pName">A pointer to a structure that receives the display name.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName) override;

    /// <summary>
    /// Sets the display name of an item in the folder.
    /// </summary>
    /// <param name="hwnd">A handle to the owner window for any UI that may be displayed.</param>
    /// <param name="pidl">The item ID of the item.</param>
    /// <param name="pszName">The new display name for the item.</param>
    /// <param name="uFlags">Flags that specify the type of display name to set.</param>
    /// <param name="ppidlOut">A pointer to the new item ID of the item.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName,
        SHGDNF uFlags, PITEMID_CHILD* ppidlOut) override;

private:

    HRESULT GetProviderCLSID(CLSID& clsidProvider) const;
};
