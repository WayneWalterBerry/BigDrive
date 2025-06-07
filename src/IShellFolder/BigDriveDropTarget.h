// <copyright file="BigDriveDropTarget.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "pch.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"
#include <windows.h>
#include <shlobj.h>

/// <summary>
/// Implements the IDropTarget interface for BigDrive shell folder extensions.
/// Handles drag-and-drop operations, including format validation and drop processing.
/// </summary>
class BigDriveDropTarget : public IDropTarget
{
private:
    /// <summary>
    /// Reference count for COM lifetime management.
    /// </summary>
    LONG m_cRef;

    /// <summary>
    /// Pointer to the parent shell folder.
    /// </summary>
    class BigDriveShellFolder* m_pFolder;

    /// <summary>
    /// Indicates if the current drag operation is allowed.
    /// </summary>
    BOOL m_fAllowDrop;

    /// <summary>
    /// Current drop effect.
    /// </summary>
    DWORD m_dwEffect;

    /// <summary>
    /// Logger for tracking events.
    /// </summary>
    BigDriveShellFolderTraceLogger m_traceLogger;

public:

    /// <summary>
    /// Initializes a new instance of the <see cref="BigDriveDropTarget"/> class.
    /// </summary>
    /// <param name="pFolder">Pointer to the parent shell folder.</param>
    BigDriveDropTarget(class BigDriveShellFolder* pFolder);

    /// <summary>
    /// Destroys an instance of the <see cref="BigDriveDropTarget"/> class.
    /// </summary>
    ~BigDriveDropTarget();

    /// <summary>
    /// Queries for a pointer to a supported interface.
    /// </summary>
    /// <param name="riid">The interface identifier.</param>
    /// <param name="ppvObject">Receives the interface pointer.</param>
    /// <returns>S_OK if successful; otherwise, E_NOINTERFACE.</returns>
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;

    /// <summary>
    /// Increments the reference count.
    /// </summary>
    /// <returns>The new reference count.</returns>
    STDMETHODIMP_(ULONG) AddRef() override;

    /// <summary>
    /// Decrements the reference count and deletes the object if the count reaches zero.
    /// </summary>
    /// <returns>The new reference count.</returns>
    STDMETHODIMP_(ULONG) Release() override;

    /// <summary>
    /// Handles the beginning of a drag operation over this drop target.
    /// </summary>
    /// <param name="pDataObj">Pointer to the IDataObject containing the dragged data.</param>
    /// <param name="grfKeyState">Current keyboard and mouse button state.</param>
    /// <param name="pt">Current mouse position in screen coordinates.</param>
    /// <param name="pdwEffect">Pointer to the effect of the drag operation.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

    /// <summary>
    /// Handles continued dragging over this drop target.
    /// </summary>
    /// <param name="grfKeyState">Current keyboard and mouse button state.</param>
    /// <param name="pt">Current mouse position in screen coordinates.</param>
    /// <param name="pdwEffect">Pointer to the effect of the drag operation.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

    /// <summary>
    /// Handles the drag pointer leaving the drop target.
    /// </summary>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall DragLeave() override;

    /// <summary>
    /// Handles the drop operation when items are dropped on this target.
    /// </summary>
    /// <param name="pDataObj">Pointer to the IDataObject containing the dropped data.</param>
    /// <param name="grfKeyState">Current keyboard and mouse button state.</param>
    /// <param name="pt">Drop position in screen coordinates.</param>
    /// <param name="pdwEffect">Pointer to the effect of the drop operation.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

private:

    /// <summary>
    /// Checks if the data object format is supported for drop.
    /// </summary>
    /// <param name="pDataObj">Pointer to the IDataObject.</param>
    /// <returns>TRUE if supported, FALSE otherwise.</returns>
    BOOL IsFormatSupported(IDataObject* pDataObj);

    /// <summary>
    /// Processes the drop operation.
    /// </summary>
    /// <param name="pDataObj">Pointer to the IDataObject.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT ProcessDrop(IDataObject* pDataObj);
};