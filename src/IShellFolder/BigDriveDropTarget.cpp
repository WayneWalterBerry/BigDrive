// <copyright file="BigDriveDropTarget.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveDropTarget.h"

#include "..\BigDrive.Client\DriveConfiguration.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "..\BigDrive.Client\Interfaces\IBigDriveFileOperations.h"

/// <summary>
/// Constructor for BigDriveDropTarget.
/// </summary>
/// <param name="pFolder">Pointer to the parent shell folder.</param>
BigDriveDropTarget::BigDriveDropTarget(BigDriveShellFolder* pFolder)
    : m_cRef(1), m_pFolder(pFolder), m_fAllowDrop(FALSE), m_dwEffect(0)
{
    m_traceLogger.Initialize(pFolder->GetDriveGuid());

    // AddRef the folder object
    if (m_pFolder)
    {
        m_pFolder->AddRef();
    }
}

/// <summary>
/// Destructor for BigDriveDropTarget.
/// </summary>
BigDriveDropTarget::~BigDriveDropTarget()
{
    // Release the folder object
    if (m_pFolder)
    {
        m_pFolder->Release();
        m_pFolder = nullptr;
    }
}


/// <summary>
/// Checks if the data object contains data in a format supported by this drop target.
/// </summary>
/// <param name="pDataObj">Pointer to the IDataObject to check.</param>
/// <returns>TRUE if the format is supported; FALSE otherwise.</returns>
BOOL BigDriveDropTarget::IsFormatSupported(IDataObject* pDataObj)
{
    BOOL bSupported = FALSE;
    FORMATETC* pfmte = nullptr;
    HRESULT hr = E_FAIL;
    CLIPFORMAT cfShellIdList = 0;
    CLIPFORMAT cfFileDescriptor = 0;
    CLIPFORMAT cfFileContents = 0;

    if (pDataObj == nullptr)
    {
        goto End;
    }

    pfmte = new (std::nothrow) FORMATETC;
    if (pfmte == nullptr)
    {
        goto End;
    }

    pfmte->cfFormat = 0;
    pfmte->ptd = nullptr;
    pfmte->dwAspect = DVASPECT_CONTENT;
    pfmte->lindex = -1;
    pfmte->tymed = TYMED_HGLOBAL;

    // Check for Shell IDList Array format
    cfShellIdList = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
    pfmte->cfFormat = cfShellIdList;
    hr = pDataObj->QueryGetData(pfmte);
    if (SUCCEEDED(hr))
    {
        bSupported = TRUE;
        goto End;
    }

    // Check for standard file drop format (HDROP)
    pfmte->cfFormat = CF_HDROP;
    hr = pDataObj->QueryGetData(pfmte);
    if (SUCCEEDED(hr))
    {
        bSupported = TRUE;
        goto End;
    }

    // Check for FileGroupDescriptor format (used by virtual files)
    cfFileDescriptor = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
    pfmte->cfFormat = cfFileDescriptor;
    hr = pDataObj->QueryGetData(pfmte);
    if (SUCCEEDED(hr))
    {
        // Also need FileContents
        cfFileContents = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
        pfmte->cfFormat = cfFileContents;
        hr = pDataObj->QueryGetData(pfmte);
        if (SUCCEEDED(hr))
        {
            bSupported = TRUE;
            goto End;
        }
    }

End:

    if (pfmte != nullptr)
    {
        delete pfmte;
        pfmte = nullptr;
    }

    return bSupported;
}

/// <summary>
/// Processes the data in the data object for drop operations.
/// </summary>
/// <param name="pDataObj">Pointer to the IDataObject containing the dropped data.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT BigDriveDropTarget::ProcessDrop(IDataObject* pDataObj)
{
    HRESULT hr = E_FAIL;
    FORMATETC fmtec = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmed = {};
    DriveConfiguration driveConfig;
    BigDriveInterfaceProvider* pProvider = nullptr;
    IBigDriveFileOperations* pFileOps = nullptr;
    PIDLIST_ABSOLUTE pidlFolder = nullptr;
    BSTR bstrTargetFolder = nullptr;
    HDROP hDrop = nullptr;
    CLSID driveGuid = GUID_NULL;
    UINT fileCount = 0;

    // Try to handle CF_HDROP (standard file drag-drop)
    fmtec.cfFormat = CF_HDROP;
    if (FAILED(pDataObj->QueryGetData(&fmtec)))
    {
        // Try Shell IDList format
        fmtec.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        if (FAILED(pDataObj->QueryGetData(&fmtec)))
        {
            hr = E_FAIL;
            goto End;
        }
        // Not implemented
        hr = E_NOTIMPL;
        goto End;
    }

    hr = pDataObj->GetData(&fmtec, &stgmed);
    if (FAILED(hr))
    {
        goto End;
    }

    hDrop = static_cast<HDROP>(stgmed.hGlobal);
    if (!hDrop)
    {
        hr = E_FAIL;
        goto End;
    }

    fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
    if (fileCount == 0)
    {
        hr = E_FAIL;
        goto End;
    }

    hr = m_pFolder->GetCurFolder(&pidlFolder);
    if (FAILED(hr) || !pidlFolder)
    {
        goto End;
    }

    hr = m_pFolder->GetPathForProviders(pidlFolder, bstrTargetFolder);
    if (FAILED(hr) || !bstrTargetFolder)
    {
        goto End;
    }

    hr = m_pFolder->GetProviderCLSID(driveGuid);
    if (FAILED(hr))
    {
        goto End;
    }

    hr = BigDriveConfigurationClient::GetDriveConfiguration(driveGuid, driveConfig);
    if (FAILED(hr))
    {
        goto End;
    }

    pProvider = new (std::nothrow) BigDriveInterfaceProvider(driveConfig);
    if (pProvider == nullptr)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    hr = pProvider->GetIBigDriveFileOperations(&pFileOps);
    if (FAILED(hr) || pFileOps == nullptr)
    {
        goto End;
    }

    // Process each dropped file
    for (UINT i = 0; i < fileCount && SUCCEEDED(hr); i++)
    {
        WCHAR filePath[MAX_PATH] = {};
        UINT cch = DragQueryFile(hDrop, i, filePath, ARRAYSIZE(filePath));
        if (cch > 0 && cch < MAX_PATH)
        {
            hr = pFileOps->CopyFileToBigDrive(driveGuid, filePath, bstrTargetFolder);
            if (FAILED(hr))
            {
                goto End;
            }
        }
    }

End:

    if (pFileOps)
    {
        pFileOps->Release();
        pFileOps = nullptr;
    }

    if (pProvider)
    {
        delete pProvider;
        pProvider = nullptr;
    }

    if (bstrTargetFolder)
    {
        SysFreeString(bstrTargetFolder);
        bstrTargetFolder = nullptr;
    }

    if (pidlFolder)
    {
        ILFree(pidlFolder);
        pidlFolder = nullptr;
    }

    if (stgmed.pUnkForRelease)
    {
        stgmed.pUnkForRelease->Release();
        stgmed.pUnkForRelease = nullptr;
    }

    if (stgmed.hGlobal)
    {
        ReleaseStgMedium(&stgmed);
        stgmed.hGlobal = nullptr;
    }

    return hr;
}