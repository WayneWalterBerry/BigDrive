// <copyright file="BigDriveDropTarget-IDropTarget.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolder.h"
#include "BigDriveDropTarget.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "..\BigDrive.Client\DriveConfiguration.h"
#include "..\BigDrive.Client\Interfaces\IBigDriveFileOperations.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"

// Clipboard format names
#define CFSTR_SHELLIDLIST      TEXT("Shell IDList Array")
#define CFSTR_FILECONTENTS     TEXT("FileContents")

/// <inheritdoc />
HRESULT __stdcall BigDriveDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    /// <inheritdoc />
    HRESULT hr = S_OK;
    BOOL fFormatSupported = FALSE;

    m_traceLogger.LogEnter(__FUNCTION__);

    if (pDataObj == nullptr || pdwEffect == nullptr)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    m_fAllowDrop = FALSE;
    m_dwEffect = DROPEFFECT_NONE;
    *pdwEffect = DROPEFFECT_NONE;

    fFormatSupported = IsFormatSupported(pDataObj);
    m_fAllowDrop = fFormatSupported;

    if (m_fAllowDrop)
    {
		m_traceLogger.LogInfo("Data object format is supported for drop.");

        m_dwEffect = DROPEFFECT_COPY;

        if ((grfKeyState & MK_CONTROL) != 0)
        {
            m_dwEffect = DROPEFFECT_COPY;
        }
        else if ((grfKeyState & MK_SHIFT) != 0)
        {
            m_dwEffect = DROPEFFECT_MOVE;
        }

        *pdwEffect = *pdwEffect & m_dwEffect;
        if (*pdwEffect == 0)
        {
            *pdwEffect = m_dwEffect;
        }
    }
    else
    {
		m_traceLogger.LogInfo("Data object format is NOT supported for drop.");
    }

End:

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    /// <inheritdoc />
    HRESULT hr = S_OK;

    m_traceLogger.LogEnter(__FUNCTION__, hr);

    if (pdwEffect == nullptr)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    *pdwEffect = DROPEFFECT_NONE;

    if (!m_fAllowDrop)
    {
        goto End;
    }

    m_dwEffect = DROPEFFECT_COPY;

    if ((grfKeyState & MK_CONTROL) != 0)
    {
        m_dwEffect = DROPEFFECT_COPY;
    }
    else if ((grfKeyState & MK_SHIFT) != 0)
    {
        m_dwEffect = DROPEFFECT_MOVE;
    }

    *pdwEffect = *pdwEffect & m_dwEffect;
    if (*pdwEffect == 0)
    {
        *pdwEffect = m_dwEffect;
    }

End:

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDropTarget::DragLeave()
{
    /// <inheritdoc />
    HRESULT hr = S_OK;

    m_traceLogger.LogEnter(__FUNCTION__);

    m_fAllowDrop = FALSE;
    m_dwEffect = DROPEFFECT_NONE;

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    /// <inheritdoc />
    HRESULT hr = S_OK;
	PIDLIST_ABSOLUTE pidlTargetFolder = nullptr;

    m_traceLogger.LogEnter(__FUNCTION__);

    if (pDataObj == nullptr || pdwEffect == nullptr)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    *pdwEffect = DROPEFFECT_NONE;

    if (!m_fAllowDrop)
    {
        goto End;
    }

    hr = ProcessDrop(pDataObj);
    if (FAILED(hr))
    {
        goto End;
    }

    *pdwEffect = m_dwEffect;

	hr = m_pFolder->GetPidlAbsolute(pidlTargetFolder);
    if (FAILED(hr))
    {
        goto End;
	}   

    // Notify the shell that the folder contents have changed
    ::SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, pidlTargetFolder, nullptr);

End:

    if (pidlTargetFolder != nullptr)
    {
        ::ILFree(pidlTargetFolder);
        pidlTargetFolder = nullptr;
	}

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}
