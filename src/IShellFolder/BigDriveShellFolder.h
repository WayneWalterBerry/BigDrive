// <copyright file="BigDriveShellFolder.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <shlobj.h> // For IShellFolder and related interfaces
#include <objbase.h> // For COM initialization
#include <string>

// Local
#include "ILExtensions.h"

/// <summary>
/// Represents a custom implementation of the IShellFolder interface for the BigDrive namespace.
/// Provides functionality for interacting with the shell folder hierarchy.
/// </summary>
class BigDriveShellFolder : public 
    IShellFolder,
	IPersistFolder2, // IPersistFolder is deprecated, use IPersistFolder2 instead
    IObjectWithBackReferences,
    IProvideClassInfo

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IPersistFolder methods

    /// <summary>
    /// Retrieves the class identifier (CLSID) for this Shell Folder extension.
    /// This allows the Windows Shell to uniquely identify the custom IShellFolder implementation.
    /// For namespace extensions, this CLSID is used during registration and binding.
    /// </summary>
    /// <param name="pClassID">Pointer to a CLSID that receives the class identifier.</param>
    /// <returns>S_OK if successful; E_POINTER if pClassID is null.</returns>
    HRESULT __stdcall GetClassID(CLSID* pClassID) override;

    /// <summary>
    /// Initializes the Shell Folder extension with its absolute location in the Shell namespace.
    /// The Shell calls this method after creating the folder object, passing the absolute PIDL.
    /// This enables the extension to know its position in the namespace hierarchy and is required
    /// for correct operation of IShellFolder extensions.
    /// </summary>
    /// <param name="pidl">The absolute PIDL that identifies the folder's location.</param>
    /// <returns>S_OK if successful; E_INVALIDARG if pidl is null; E_OUTOFMEMORY if cloning fails.</returns>
    HRESULT __stdcall Initialize(PCIDLIST_ABSOLUTE pidl) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IPersistFolder2 methods

    /// <summary>
    /// Retrieves the current absolute PIDL for this Shell Folder.
    /// This is required for IShellFolder extensions to allow the shell to query the folder's location.
    /// </summary>
    /// <param name="ppidl">Address of a pointer that receives the PIDL. The caller is responsible for freeing it with ILFree.</param>
    /// <returns>S_OK if successful; E_POINTER if ppidl is null; E_OUTOFMEMORY if cloning fails.</returns>
    HRESULT __stdcall GetCurFolder(PIDLIST_ABSOLUTE* ppidl) override;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IObjectWithBackReferences methods
    
    /// <summary>
    /// Returns the number of back references currently held by this shell folder object.
    /// The Windows Shell may call this method to determine if the folder object is referenced
    /// by other components or objects, which can affect resource management and object lifetime.
    /// 
    /// In the context of a shell namespace extension, a "back reference" typically means another
    /// object (such as a view, enumerator, or child item) is holding a reference to this folder.
    /// If the count is nonzero, the shell may delay releasing or destroying the folder object to
    /// avoid breaking those references.
    /// 
    /// This minimal implementation always returns 0, indicating that the folder does not track
    /// or expose any back references. As a result, the shell will treat the object as having no
    /// outstanding dependencies and may release it as soon as its own reference count drops to zero.
    /// </summary> 
    /// <param name="pcRef">Pointer to a ULONG that receives the back reference count.</param>
    /// <returns>S_OK if successful; E_POINTER if <paramref name="pcRef"/> is null.</returns>
    /// <remarks>
    /// Returning 0 is appropriate for most shell extensions that do not implement custom
    /// back reference tracking. If your extension does manage such relationships, return
    /// the actual count instead.
    /// </remarks>
    HRESULT __stdcall GetBackReferencesCount(ULONG* pcRef);

    /// <summary>
    /// Removes all back references currently held by this shell folder object.
    /// The Windows Shell may call this method to instruct the folder to release or clear
    /// any objects or resources that are considered "back references." In the context of
    /// a shell namespace extension, a back reference typically means another object
    /// (such as a view, enumerator, or child item) is holding a reference to this folder,
    /// and the shell wants to ensure those references are released to allow for cleanup
    /// or object destruction.
    /// 
    /// This minimal implementation does nothing and always returns S_OK, indicating that
    /// the folder does not track or manage any back references. This is appropriate for
    /// most shell extensions that do not implement custom back reference tracking logic.
    /// If your extension does manage such relationships, you should release or clear
    /// those references here.
    /// </summary>
    /// <returns>S_OK to indicate success.</returns>
    /// <remarks>
    /// Returning S_OK with no action is the standard approach for shell extensions that
    /// do not need to manage back references. If you add custom reference tracking,
    /// update this method to perform the necessary cleanup.
    /// </remarks>
    HRESULT __stdcall RemoveBackReferences();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IProvideClassInfo methods

    /// <summary>
    /// Retrieves the ITypeInfo interface for the class.
    /// The Windows Shell or other clients may call this method to obtain type information
    /// for automation or introspection purposes. In the context of a shell folder extension,
    /// this is rarely needed, so a minimal implementation simply returns E_NOTIMPL.
    /// </summary>
    /// <param name="ppTI">Address of a pointer that receives the ITypeInfo interface pointer.</param>
    /// <returns>E_NOTIMPL to indicate that type information is not provided.</returns>
    /// <remarks>
    /// Returning E_NOTIMPL is standard for shell extensions that do not expose automation
    /// or type information. If you need to provide type info for scripting or automation,
    /// implement this method to return a valid ITypeInfo pointer.
    /// </remarks>
    HRESULT __stdcall GetClassInfo(ITypeInfo** ppTI);

private:

    HRESULT GetProviderCLSID(CLSID& clsidProvider) const;
};
