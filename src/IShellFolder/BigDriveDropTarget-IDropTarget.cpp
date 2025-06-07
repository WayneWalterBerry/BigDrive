// <copyright file="BigDriveDropTarget-IDropTarget.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveDropTarget.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "..\BigDrive.Client\DriveConfiguration.h"
#include "..\BigDrive.Client\Interfaces\IBigDriveFileOperations.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"

// Clipboard format names
#define CFSTR_SHELLIDLIST      TEXT("Shell IDList Array")
#define CFSTR_FILECONTENTS     TEXT("FileContents")

/// <summary>
/// Handles the beginning of a drag operation over this drop target.
/// </summary>
/// <param name="pDataObj">Pointer to the IDataObject containing the dragged data.</param>
/// <param name="grfKeyState">Current keyboard and mouse button state.</param>
/// <param name="pt">Current mouse position in screen coordinates.</param>
/// <param name="pdwEffect">Pointer to the effect of the drag operation.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT __stdcall BigDriveDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (!pDataObj || !pdwEffect)
        return E_INVALIDARG;

    // Default behavior - don't allow drop
    m_fAllowDrop = FALSE;
    m_dwEffect = DROPEFFECT_NONE;
    *pdwEffect = DROPEFFECT_NONE;

    // Check if the data format is supported
    m_fAllowDrop = IsFormatSupported(pDataObj);

    // If the format is supported, determine the allowed effect
    if (m_fAllowDrop)
    {
        // Default to copy for BigDrive targets
        m_dwEffect = DROPEFFECT_COPY;

        // Update the effect based on keyboard modifiers
        if (grfKeyState & MK_CONTROL)
            m_dwEffect = DROPEFFECT_COPY;
        else if (grfKeyState & MK_SHIFT)
            m_dwEffect = DROPEFFECT_MOVE;

        // Limit to what's allowed by the source
        *pdwEffect = *pdwEffect & m_dwEffect;
        if (*pdwEffect == 0)
            *pdwEffect = m_dwEffect;
    }

    return S_OK;
}

/// <summary>
/// Handles continued dragging over this drop target.
/// </summary>
/// <param name="grfKeyState">Current keyboard and mouse button state.</param>
/// <param name="pt">Current mouse position in screen coordinates.</param>
/// <param name="pdwEffect">Pointer to the effect of the drag operation.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT __stdcall BigDriveDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (!pdwEffect)
        return E_INVALIDARG;

    *pdwEffect = DROPEFFECT_NONE;

    // If drop isn't allowed, stop here
    if (!m_fAllowDrop)
        return S_OK;

    // Default to copy for BigDrive targets
    m_dwEffect = DROPEFFECT_COPY;

    // Update the effect based on keyboard modifiers
    if (grfKeyState & MK_CONTROL)
        m_dwEffect = DROPEFFECT_COPY;
    else if (grfKeyState & MK_SHIFT)
        m_dwEffect = DROPEFFECT_MOVE;

    // Limit to what's allowed by the source
    *pdwEffect = *pdwEffect & m_dwEffect;
    if (*pdwEffect == 0)
        *pdwEffect = m_dwEffect;

    return S_OK;
}

/// <summary>
/// Handles the drag pointer leaving the drop target.
/// </summary>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT __stdcall BigDriveDropTarget::DragLeave()
{
    // Reset drag state
    m_fAllowDrop = FALSE;
    m_dwEffect = DROPEFFECT_NONE;

    return S_OK;
}

/// <summary>
/// Handles the drop operation when items are dropped on this target.
/// </summary>
/// <param name="pDataObj">Pointer to the IDataObject containing the dropped data.</param>
/// <param name="grfKeyState">Current keyboard and mouse button state.</param>
/// <param name="pt">Drop position in screen coordinates.</param>
/// <param name="pdwEffect">Pointer to the effect of the drop operation.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT __stdcall BigDriveDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    HRESULT hr = S_OK;

    if (!pDataObj || !pdwEffect)
        return E_INVALIDARG;

    // Default to no effect
    *pdwEffect = DROPEFFECT_NONE;

    // Check if drop is allowed
    if (!m_fAllowDrop)
        return S_OK;

    // Process the drop
    hr = ProcessDrop(pDataObj);
    if (SUCCEEDED(hr))
    {
        *pdwEffect = m_dwEffect;
    }

    return hr;
}
