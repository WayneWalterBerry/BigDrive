// <copyright file="BigDriveTransferSource.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <ShlObj.h>
#include <objidl.h>
#include "BigDriveShellFolder.h"

/// <summary>
/// Implements the ITransferSource interface to support file operations in the BigDrive shell namespace.
/// This interface is used by the shell to perform file transfer operations such as copying and moving items.
/// </summary>
class BigDriveTransferSource : public ITransferSource
{

private:

    /// <summary>
    /// Reference count for COM object lifetime management.
    /// </summary>
    long m_cRef;

    /// <summary>
    /// Pointer to the parent shell folder that owns this transfer source.
    /// </summary>
    BigDriveShellFolder* m_pFolder;

    /// <summary>
    /// Logger for tracking events.
    /// </summary>
    BigDriveShellFolderTraceLogger m_traceLogger;

    /// <summary>
    /// The Drive GUID for this transfer source.
    /// </summary>
    GUID m_driveGuid;

public:

    /// <summary>
    /// Constructs a new instance of the BigDriveTransferSource class.
    /// </summary>
    /// <param name="pFolder">The BigDriveShellFolder that owns this transfer source.</param>
    BigDriveTransferSource(BigDriveShellFolder* pFolder);

    /// <summary>
    /// Destructor for BigDriveTransferSource.
    /// </summary>
    virtual ~BigDriveTransferSource();

    // IUnknown methods

    /// <summary>
    /// Queries for a specified interface on the object.
    /// </summary>
    /// <param name="riid">The identifier of the interface being requested.</param>
    /// <param name="ppv">Address of pointer variable that receives the interface pointer if successful.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

    /// <summary>
    /// Increments the reference count for the interface.
    /// </summary>
    /// <returns>The new reference count.</returns>
    STDMETHODIMP_(ULONG) AddRef();

    /// <summary>
    /// Decrements the reference count for the interface.
    /// </summary>
    /// <returns>The new reference count.</returns>
    STDMETHODIMP_(ULONG) Release();

    // ITransferSource methods

    /// <summary>
    /// Registers a callback interface to receive notifications during transfer operations.
    /// </summary>
    /// <param name="psink">Pointer to the ITransferAdviseSink interface.</param>
    /// <param name="pdwCookie">Receives a token identifying the callback registration.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP Advise(ITransferAdviseSink* psink, DWORD* pdwCookie);

    /// <summary>
    /// Cancels a callback registration previously established with Advise.
    /// </summary>
    /// <param name="dwCookie">Token identifying the callback registration.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP Unadvise(DWORD dwCookie);

    /// <summary>
    /// Sets properties to be applied to items during transfer operations.
    /// </summary>
    /// <param name="pproparray">Array of property change operations.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP SetProperties(IPropertyChangeArray* pproparray);

    /// <summary>
    /// Opens an item for a transfer operation.
    /// </summary>
    /// <param name="psi">The shell item to open.</param>
    /// <param name="flags">Flags controlling the operation.</param>
    /// <param name="riid">Interface identifier for the returned interface.</param>
    /// <param name="ppv">Receives the requested interface pointer.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP OpenItem(IShellItem* psi, TRANSFER_SOURCE_FLAGS flags, REFIID riid, void** ppv);

    /// <summary>
    /// Moves an item to a new location.
    /// </summary>
    /// <param name="psi">The shell item to move.</param>
    /// <param name="psiParentDst">The destination parent folder.</param>
    /// <param name="pszNameDst">The new name for the item, or NULL to keep the existing name.</param>
    /// <param name="flags">Flags controlling the move operation.</param>
    /// <param name="ppsiNew">Receives the new shell item after the move.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP MoveItem(IShellItem* psi, IShellItem* psiParentDst, LPCWSTR pszNameDst,
        TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNew);

    /// <summary>
    /// Sends an item to the recycle bin.
    /// </summary>
    /// <param name="psiSource">The shell item to recycle.</param>
    /// <param name="psiParentDest">The destination parent folder (typically the recycle bin).</param>
    /// <param name="flags">Flags controlling the recycle operation.</param>
    /// <param name="ppsiNewDest">Receives the new shell item in its recycled location.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP RecycleItem(IShellItem* psiSource, IShellItem* psiParentDest,
        TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);

    /// <summary>
    /// Permanently removes an item.
    /// </summary>
    /// <param name="psiSource">The shell item to remove.</param>
    /// <param name="flags">Flags controlling the remove operation.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP RemoveItem(IShellItem* psiSource, TRANSFER_SOURCE_FLAGS flags);

    /// <summary>
    /// Renames an item without changing its location.
    /// </summary>
    /// <param name="psi">The shell item to rename.</param>
    /// <param name="pszNewName">The new name for the item.</param>
    /// <param name="flags">Flags controlling the rename operation.</param>
    /// <param name="ppsiNew">Receives the shell item with its new name.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP RenameItem(IShellItem* psi, LPCWSTR pszNewName,
        TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNew);

    /// <summary>
    /// Creates a link (shortcut) to an item.
    /// </summary>
    /// <param name="psiSource">The shell item to link to.</param>
    /// <param name="psiParentDest">The destination parent folder for the link.</param>
    /// <param name="pszName">The name for the link, or NULL to use a default name.</param>
    /// <param name="flags">Flags controlling the link creation.</param>
    /// <param name="ppsiNewDest">Receives the new shell item representing the link.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP LinkItem(IShellItem* psiSource, IShellItem* psiParentDest,
        LPCWSTR pszName, TRANSFER_SOURCE_FLAGS flags, IShellItem** ppsiNewDest);

    /// <summary>
    /// Applies previously set properties to an item.
    /// </summary>
    /// <param name="psiSource">The shell item to update with properties.</param>
    /// <param name="ppsiNew">Receives the updated shell item.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP ApplyPropertiesToItem(IShellItem* psiSource, IShellItem** ppsiNew);

    /// <summary>
    /// Gets the default name that would be used for an item in a destination.
    /// </summary>
    /// <param name="psiSource">The source shell item.</param>
    /// <param name="psiParentDest">The destination parent folder.</param>
    /// <param name="ppszDestinationName">Receives the default destination name.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP GetDefaultDestinationName(IShellItem* psiSource, IShellItem* psiParentDest,
        LPWSTR* ppszDestinationName);

    /// <summary>
    /// Called when entering a folder during a recursive transfer operation.
    /// </summary>
    /// <param name="psiChildFolderDest">The destination folder being entered.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP EnterFolder(IShellItem* psiChildFolderDest);

    /// <summary>
    /// Called when leaving a folder during a recursive transfer operation.
    /// </summary>
    /// <param name="psiChildFolderDest">The destination folder being left.</param>
    /// <returns>S_OK if successful, or an error code if not.</returns>
    STDMETHODIMP LeaveFolder(IShellItem* psiChildFolderDest);

private:

    /// <summary>
    /// Helper method to handle property failures during transfer operations.
    /// </summary>
    /// <param name="psi">The shell item that failed.</param>
    /// <param name="key">The property key that failed.</param>
    /// <param name="hrFailure">The HRESULT of the failure.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT PropertyFailure(IShellItem* psi, REFPROPERTYKEY key, HRESULT hrFailure);
};