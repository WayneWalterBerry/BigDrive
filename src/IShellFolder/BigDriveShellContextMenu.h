// <copyright file="BigDriveShellContextMenu.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shlobj.h>
#include <comdef.h>
#include <memory>
#include "BigDriveShellFolder.h"

/// <summary>
/// Implementation of IContextMenu for the BigDrive shell namespace extension.
/// Provides standard and custom context menu items for BigDrive items.
/// </summary>
class BigDriveShellContextMenu :
    public IContextMenu3
{
private:

    /// <summary>
    /// Logger that captures trace information for the shell folder.
    /// </summary>
    BigDriveShellFolderTraceLogger m_traceLogger;

public:

    /// <summary>
    /// Factory method to create an instance of BigDriveContextMenu.
    /// </summary>
    /// <param name="pFolder">The parent BigDriveShellFolder object.</param>
    /// <param name="cidl">Count of item IDs in the array.</param>
    /// <param name="apidl">Array of item IDs for which to create the context menu.</param>
    /// <param name="ppv">On success, receives the IContextMenu interface pointer.</param>
    /// <returns>S_OK if successful, or an error code.</returns>
    static HRESULT CreateInstance(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, void** ppv);

private:

    /// <summary>
    /// Private constructor - use CreateInstance to create instances.
    /// </summary>
    /// <param name="pFolder">The parent BigDriveShellFolder object.</param>
    /// <param name="cidl">Count of item IDs in the array.</param>
    /// <param name="apidl">Array of item IDs for which to create the context menu.</param>
    BigDriveShellContextMenu(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl);
    
    /// <summary>
    /// Destructor.
    /// </summary>
    ~BigDriveShellContextMenu();

public:
    
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

public: // IContextMenu methods
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax);

public: // IContextMenu2 methods
    STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);

public: // IContextMenu3 methods
    STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

private:
    /// <summary>
    /// Gets the shell's default context menu handler for the selected items.
    /// </summary>
    /// <param name="ppContextMenu">On success, receives the IContextMenu interface pointer.</param>
    /// <returns>S_OK if successful, or an error code.</returns>
    HRESULT GetShellContextMenu(IContextMenu** ppContextMenu);
    
    /// <summary>
    /// Initializes the context menu with BigDrive-specific commands.
    /// </summary>
    /// <param name="hmenu">The menu handle to populate.</param>
    /// <param name="indexMenu">The position at which to insert the first menu item.</param>
    /// <param name="idCmdFirst">The minimum value that can be used for a menu item ID.</param>
    /// <param name="idCmdLast">The maximum value that can be used for a menu item ID.</param>
    /// <returns>The number of menu items added.</returns>
    UINT AddBigDriveMenuItems(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast);

private:
    // Reference count
    LONG m_cRef;
    
    // Parent folder
    BigDriveShellFolder* m_pFolder;
    
    // Count of selected items
    UINT m_cidl;
    
    // Array of selected item IDs
    std::unique_ptr<PCITEMID_CHILD[]> m_apidl;
    
    // IDs for our custom commands
    enum {
        ID_OPEN = 0,
        ID_PROPERTIES,
        ID_REFRESH,
        ID_CUSTOM_COMMAND,
        ID_CUSTOM_COUNT
    };
    
    // First command ID (set during QueryContextMenu)
    UINT m_idCmdFirst;
    
    // Shell's context menu handler if we're using it
    IContextMenu* m_pShellContextMenu;
    
    // Cached verbs for commands
    WCHAR m_szVerbs[ID_CUSTOM_COUNT][32];
};