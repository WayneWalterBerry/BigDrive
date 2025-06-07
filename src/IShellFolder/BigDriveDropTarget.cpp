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
    if (!pDataObj)
        return FALSE;

    // Check for Shell IDList Array format
    FORMATETC fmte = {
        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST),
        nullptr,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    if (SUCCEEDED(pDataObj->QueryGetData(&fmte)))
        return TRUE;

    // Check for standard file drop format (HDROP)
    fmte.cfFormat = CF_HDROP;
    if (SUCCEEDED(pDataObj->QueryGetData(&fmte)))
        return TRUE;

    // Check for FileGroupDescriptor format (used by virtual files)
    fmte.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
    if (SUCCEEDED(pDataObj->QueryGetData(&fmte)))
    {
        // Also need FileContents
        fmte.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
        if (SUCCEEDED(pDataObj->QueryGetData(&fmte)))
            return TRUE;
    }

    return FALSE;
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
    STGMEDIUM stgmed;
    DriveConfiguration driveConfig;
    BigDriveInterfaceProvider* pProvider = nullptr;
    IBigDriveFileOperations* pFileOps = nullptr;
    PIDLIST_ABSOLUTE pidlFolder = nullptr;
    BSTR bstrTargetFolder = nullptr;

    // First try to handle CF_HDROP (standard file drag-drop)
    fmtec.cfFormat = CF_HDROP;
    if (SUCCEEDED(pDataObj->QueryGetData(&fmtec)))
    {
        hr = pDataObj->GetData(&fmtec, &stgmed);
        if (SUCCEEDED(hr))
        {
            HDROP hDrop = static_cast<HDROP>(stgmed.hGlobal);
            if (hDrop)
            {
                // Get number of files dropped
                UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
                if (fileCount > 0)
                {
                    // Get target folder path
                    hr = m_pFolder->GetCurFolder(&pidlFolder);
                    if (FAILED(hr) || !pidlFolder)
                    {
                        ReleaseStgMedium(&stgmed);
                        return hr;
                    }

                    hr = m_pFolder->GetPathForProviders(pidlFolder, bstrTargetFolder);
                    if (FAILED(hr) || !bstrTargetFolder)
                    {
                        ILFree(pidlFolder);
                        ReleaseStgMedium(&stgmed);
                        return hr;
                    }

                    // Get the BigDrive interface 
                    CLSID driveGuid;
                    hr = m_pFolder->GetProviderCLSID(driveGuid);
                    if (SUCCEEDED(hr))
                    {
                        hr = BigDriveConfigurationClient::GetDriveConfiguration(driveGuid, driveConfig);
                        if (SUCCEEDED(hr))
                        {
                            pProvider = new BigDriveInterfaceProvider(driveConfig);
                            if (pProvider)
                            {
                                hr = pProvider->GetIBigDriveFileOperations(&pFileOps);
                                if (SUCCEEDED(hr) && pFileOps)
                                {
                                    // Process each dropped file
                                    for (UINT i = 0; i < fileCount && SUCCEEDED(hr); i++)
                                    {
                                        WCHAR filePath[MAX_PATH];
                                        UINT cch = DragQueryFile(hDrop, i, filePath, ARRAYSIZE(filePath));
                                        if (cch > 0 && cch < MAX_PATH)
                                        {
                                            // Copy the file to BigDrive
                                            hr = pFileOps->CopyFileToBigDrive(driveGuid, filePath, bstrTargetFolder);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
            }

            ReleaseStgMedium(&stgmed);
        }
    }
    else
    {
        // Try Shell IDList format
        fmtec.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        if (SUCCEEDED(pDataObj->QueryGetData(&fmtec)))
        {
            // This would require more complex handling to extract PIDLs and process them
            // For a complete implementation, we would extract the PIDLs, get their file paths,
            // and use the BigDrive file operations interface to copy/move them
            hr = E_NOTIMPL; // Not implemented in this example
        }
    }

    // Clean up resources
    if (pFileOps)
        pFileOps->Release();

    if (pProvider)
        delete pProvider;

    if (bstrTargetFolder)
        SysFreeString(bstrTargetFolder);

    if (pidlFolder)
        ILFree(pidlFolder);

    return hr;
}