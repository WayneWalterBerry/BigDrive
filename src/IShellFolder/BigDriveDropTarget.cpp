// <copyright file="BigDriveDropTarget.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveDropTarget.h"

#include "BigDriveShellFolder.h"

#include "..\BigDrive.Client\DriveConfiguration.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "..\BigDrive.Client\Interfaces\IBigDriveFileOperations.h"

#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <shlobj.h>

CLIPFORMAT g_cfShellIdList = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_SHELLIDLIST);
CLIPFORMAT g_cfFileDescriptor = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT g_cfFileContents = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT g_cfHDrop = CF_HDROP;

/// <summary>
/// Constructor for BigDriveDropTarget.
/// </summary>
/// <param name="pFolder">Pointer to the parent shell folder.</param>
BigDriveDropTarget::BigDriveDropTarget(BigDriveShellFolder* pFolder)
	: m_cRef(1), m_pFolder(pFolder), m_fAllowDrop(FALSE), m_dwEffect(0), m_traceLogger()
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

	m_traceLogger.Uninitialize();
}

/// <summary>
/// Checks if the data object contains data in a format supported by this drop target.
/// </summary>
/// <param name="pDataObj">Pointer to the IDataObject to check.</param>
/// <returns>TRUE if the format is supported; FALSE otherwise.</returns>
BOOL BigDriveDropTarget::IsFormatSupported(IDataObject* pDataObj)
{
	BOOL bSupported = FALSE;
	HRESULT hr = E_FAIL;
	CLIPFORMAT cfShellIdList = 0;
	CLIPFORMAT cfFileDescriptor = 0;
	CLIPFORMAT cfFileContents = 0;

	if (pDataObj == nullptr)
	{
		goto End;
	}

	FORMATETC fmte;

	fmte.cfFormat = 0;
	fmte.ptd = nullptr;
	fmte.dwAspect = DVASPECT_CONTENT;
	fmte.lindex = -1;
	fmte.tymed = TYMED_HGLOBAL;

	// Check for Shell IDList Array format
	cfShellIdList = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	fmte.cfFormat = cfShellIdList;

	hr = pDataObj->QueryGetData(&fmte);
	if (SUCCEEDED(hr))
	{
		m_traceLogger.LogInfo("Data object supports CFSTR_SHELLIDLIST format.");
		bSupported = TRUE;
		goto End;
	}

	// Check for standard file drop format (HDROP)
	fmte.cfFormat = CF_HDROP;
	hr = pDataObj->QueryGetData(&fmte);
	if (SUCCEEDED(hr))
	{
		m_traceLogger.LogInfo("Data object supports CF_HDROP format.");
		bSupported = TRUE;
		goto End;
	}

	// Check for FileGroupDescriptor format (used by virtual files)
	cfFileDescriptor = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	fmte.cfFormat = cfFileDescriptor;

	hr = pDataObj->QueryGetData(&fmte);
	if (SUCCEEDED(hr))
	{
		// Also need FileContents
		cfFileContents = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
		fmte.cfFormat = cfFileContents;
		hr = pDataObj->QueryGetData(&fmte);
		if (SUCCEEDED(hr))
		{
			m_traceLogger.LogInfo("Data object supports virtual file formats (CFSTR_FILEDESCRIPTOR and CFSTR_FILECONTENTS).");
			bSupported = TRUE;
			goto End;
		}
	}

End:

	return bSupported;
}

/// <summary>
/// Processes the data in the data object for drop operations.
/// </summary>
/// <param name="pDataObj">Pointer to the IDataObject containing the dropped data.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT BigDriveDropTarget::ProcessDrop(IDataObject* pIDataObject)
{
	HRESULT hr = E_NOTIMPL;

	{
		// Try to handle CF_HDROP (standard file drag-drop)
		FORMATETC fmtec = { g_cfHDrop, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		if (SUCCEEDED(pIDataObject->QueryGetData(&fmtec)))
		{
			return ProcessHDrop(pIDataObject);
		}
	}

	{
		// Try Shell IDList format
		FORMATETC fmtec = { g_cfShellIdList, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		if (SUCCEEDED(pIDataObject->QueryGetData(&fmtec)))
		{
			return ProcessShellIdListDrop(pIDataObject);
		}
	}

	return hr;
}

/// <summary>
/// Processes an HDROP data object, copying each dropped file to a target folder on a BigDrive volume using the BigDrive file operations interface.
/// </summary>
/// <param name="pIDataObject">A pointer to an IDataObject containing the HDROP data representing the files to be processed.</param>
/// <returns>Returns an HRESULT indicating success or failure. S_OK if all files were processed successfully; otherwise, an error code describing the failure.</returns>
HRESULT BigDriveDropTarget::ProcessHDrop(IDataObject* pIDataObject)
{
	HRESULT hr = E_FAIL;
	FORMATETC fmtec = { g_cfHDrop, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed = {};
	DriveConfiguration driveConfig;
	BigDriveInterfaceProvider* pProvider = nullptr;
	IBigDriveFileOperations* pFileOps = nullptr;
	PIDLIST_ABSOLUTE pidlFolder = nullptr;
	BSTR bstrTargetFolder = nullptr;
	HDROP hDrop = nullptr;
	CLSID driveGuid = GUID_NULL;
	UINT fileCount = 0;

	hr = pIDataObject->GetData(&fmtec, &stgmed);
	if (FAILED(hr))
	{
		WriteErrorFormatted(L"Failed to get data from IDataObject, hr=0x%08X", hr);
		goto End;
	}

	hDrop = static_cast<HDROP>(stgmed.hGlobal);
	if (!hDrop)
	{
		WriteError(L"Failed to get HDROP data from storage medium");
		hr = E_FAIL;
		goto End;
	}

	fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
	if (fileCount == 0)
	{
		WriteError(L"No files found in dropped data");
		hr = E_FAIL;
		goto End;
	}

	hr = m_pFolder->GetCurFolder(&pidlFolder);
	if (FAILED(hr) || !pidlFolder)
	{
		WriteErrorFormatted(L"Failed to get current folder PIDL, hr=0x%08X", hr);
		goto End;
	}

	hr = m_pFolder->GetPathForProviders(pidlFolder, bstrTargetFolder);
	if (FAILED(hr) || !bstrTargetFolder)
	{
		WriteErrorFormatted(L"Failed to get target folder path, hr=0x%08X", hr);
		goto End;
	}

	driveGuid = m_pFolder->GetDriveGuid();

	hr = BigDriveConfigurationClient::GetDriveConfiguration(driveGuid, driveConfig);
	if (FAILED(hr))
	{
		WriteErrorFormatted(L"Failed to get drive configuration, hr=0x%08X", hr);
		goto End;
	}

	pProvider = new BigDriveInterfaceProvider(driveConfig);
	if (pProvider == nullptr)
	{
		WriteError(L"Failed to create BigDriveInterfaceProvider due to insufficient memory");
		hr = E_OUTOFMEMORY;
		goto End;
	}

	hr = pProvider->GetIBigDriveFileOperations(&pFileOps);
	if (FAILED(hr) || pFileOps == nullptr)
	{
		WriteErrorFormatted(L"Failed to get IBigDriveFileOperations interface, hr=0x%08X", hr);
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
				WriteErrorFormatted(L"Failed to copy file '%s' to BigDrive, hr=0x%08X", filePath, hr);
				goto End;
			}
		}
		else
		{
			WriteErrorFormatted(L"Failed to get file path for item %d, cch=%d", i, cch);
			hr = E_FAIL;
			goto End;
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
		::SysFreeString(bstrTargetFolder);
		bstrTargetFolder = nullptr;
	}

	if (pidlFolder)
	{
		::ILFree(pidlFolder);
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

/// <summary>
/// Processes a shell ID list drop operation using the provided data object.
/// </summary>
/// <param name="pIDataObject">Pointer to an IDataObject containing the data for the shell ID list drop.</param>
/// <returns>An HRESULT value indicating success or failure of the operation.</returns>
HRESULT BigDriveDropTarget::ProcessShellIdListDrop(IDataObject* pIDataObject)
{
	HRESULT hr = S_OK;
	FORMATETC fmtec = { g_cfShellIdList, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed = {};
	DriveConfiguration driveConfig;
	BigDriveInterfaceProvider* pProvider = nullptr;
	IBigDriveFileOperations* pFileOps = nullptr;
	PIDLIST_ABSOLUTE pidlFolder = nullptr;
	BSTR bstrTargetFolder = nullptr;
	LPIDA pida = nullptr;
	CLSID driveGuid = GUID_NULL;
	LPCITEMIDLIST pidlParent = nullptr;
	BOOL bGlobalLocked = FALSE;
	LPITEMIDLIST pidlFull = nullptr;

	hr = pIDataObject->GetData(&fmtec, &stgmed);
	if (FAILED(hr))
	{
		WriteErrorFormatted(L"Failed to get Shell ID List data from IDataObject, hr=0x%08X", hr);
		goto End;
	}

	pida = (LPIDA)::GlobalLock(stgmed.hGlobal);
	if (!pida)
	{
		WriteError(L"Failed to lock global memory for Shell ID List");
		hr = E_FAIL;
		goto End;
	}

	bGlobalLocked = TRUE;

	// Get target folder for copy/move operation
	hr = m_pFolder->GetCurFolder(&pidlFolder);
	if (FAILED(hr) || !pidlFolder)
	{
		WriteErrorFormatted(L"Failed to get current folder PIDL, hr=0x%08X", hr);
		goto End;
	}

	hr = m_pFolder->GetPathForProviders(pidlFolder, bstrTargetFolder);
	if (FAILED(hr) || !bstrTargetFolder)
	{
		WriteErrorFormatted(L"Failed to get target folder path, hr=0x%08X", hr);
		goto End;
	}

	driveGuid = m_pFolder->GetDriveGuid();

	// Setup interface provider
	hr = BigDriveConfigurationClient::GetDriveConfiguration(driveGuid, driveConfig);
	if (FAILED(hr))
	{
		WriteErrorFormatted(L"Failed to get drive configuration, hr=0x%08X", hr);
		goto End;
	}

	pProvider = new BigDriveInterfaceProvider(driveConfig);
	if (pProvider == nullptr)
	{
		WriteError(L"Failed to create BigDriveInterfaceProvider due to insufficient memory");
		hr = E_OUTOFMEMORY;
		goto End;
	}

	hr = pProvider->GetIBigDriveFileOperations(&pFileOps);
	if (FAILED(hr) || pFileOps == nullptr)
	{
		WriteErrorFormatted(L"Failed to get IBigDriveFileOperations interface, hr=0x%08X", hr);
		goto End;
	}

	// Process each item in the Shell IDList
	if (pida->cidl == 0)
	{
		WriteError(L"Shell ID List contains no items");
		hr = E_FAIL;
		goto End;
	}

	// Get the parent folder PIDL - located at offset aoffset[0]
	pidlParent = (LPCITEMIDLIST)((BYTE*)pida + pida->aoffset[0]);
	if (!pidlParent)
	{
		WriteError(L"Failed to get parent folder PIDL");
		hr = E_FAIL;
		goto End;
	}

	for (UINT i = 0; i < pida->cidl; i++)
	{
		WCHAR filePath[MAX_PATH] = {};

		// The item PIDLs start at aoffset[1] (after the parent folder)
		LPCITEMIDLIST pidlItem = (LPCITEMIDLIST)((BYTE*)pida + pida->aoffset[i + 1]);
		if (pidlItem == nullptr)
		{
			continue;
		}

		pidlFull = ::ILCombine(pidlParent, pidlItem);
		if (!pidlFull)
		{
			WriteErrorFormatted(L"Failed to combine parent and item PIDLs for item %d", i);
			hr = E_FAIL;
			goto End;
		}

		// Convert PIDL to file path
		if (!::SHGetPathFromIDList(pidlFull, filePath))
		{
			WriteErrorFormatted(L"Failed to get file path from PIDL for item %d", i);
			hr = E_FAIL;
			goto End;
		}

		hr = pFileOps->CopyFileToBigDrive(driveGuid, filePath, bstrTargetFolder);
		if (FAILED(hr))
		{
			WriteErrorFormatted(L"Failed to copy file '%s' to BigDrive, hr=0x%08X", filePath, hr);
			goto End;
		}

		if (pidlFull)
		{
			::ILFree(pidlFull);
			pidlFull = nullptr;
		}
	}

End:

	if (pidlFull)
	{
		::ILFree(pidlFull);
		pidlFull = nullptr;
	}

	// Unlock global memory if it was locked
	if (bGlobalLocked && stgmed.hGlobal)
	{
		::GlobalUnlock(stgmed.hGlobal);
		bGlobalLocked = FALSE;
	}

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
		::SysFreeString(bstrTargetFolder);
		bstrTargetFolder = nullptr;
	}

	if (pidlFolder)
	{
		::ILFree(pidlFolder);
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

/// <inheritdoc />
HRESULT BigDriveDropTarget::WriteError(LPCWSTR szMessage)
{
	m_pFolder->WriteError(szMessage);

	return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveDropTarget::WriteErrorFormatted(LPCWSTR formatter, ...)
{
	wchar_t formattedMsg[1024];
	va_list args;
	va_start(args, formatter);
	_vsnwprintf_s(formattedMsg, _countof(formattedMsg), _TRUNCATE, formatter, args);
	va_end(args);

	m_pFolder->WriteError(formattedMsg);

	return S_OK;
}