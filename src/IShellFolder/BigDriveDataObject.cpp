// <copyright file="BigDriveDataObject.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveDataObject.h"

#include "Logging\BigDriveShellFolderTraceLogger.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"
#include "..\BigDrive.Client\DriveConfiguration.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

/// <summary>
/// Constructor for BigDriveDataObject.
/// </summary>
/// <param name="pFolder">Pointer to the parent shell folder.</param>
/// <param name="cidl">Count of items.</param>
/// <param name="apidl">Array of item IDs.</param>
BigDriveDataObject::BigDriveDataObject(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl)
	: m_cRef(1), m_pFolder(pFolder), m_cidl(cidl), m_apidl(nullptr)
{
	m_traceLogger.Initialize(pFolder->GetDriveGuid());

	// AddRef the folder object
	if (m_pFolder)
	{
		m_pFolder->AddRef();
	}

	// Create a copy of the item IDs
	if (m_cidl > 0 && apidl)
	{
		m_apidl = static_cast<PCUITEMID_CHILD*>(CoTaskMemAlloc(sizeof(PCUITEMID_CHILD) * m_cidl));
		if (m_apidl)
		{
			for (UINT i = 0; i < m_cidl; i++)
			{
				m_apidl[i] = ILClone(apidl[i]);
			}
		}
	}

	m_driveGuid = m_pFolder->GetDriveGuid();
}

/// <summary>
/// Destructor for BigDriveDataObject.
/// </summary>
BigDriveDataObject::~BigDriveDataObject()
{
	// Free item IDs
	if (m_apidl)
	{
		for (UINT i = 0; i < m_cidl; i++)
		{
			if (m_apidl[i])
				ILFree((PITEMID_CHILD)m_apidl[i]);
		}
		CoTaskMemFree(m_apidl);
		m_apidl = nullptr;
	}

	// Release the folder object
	if (m_pFolder)
	{
		m_pFolder->Release();
		m_pFolder = nullptr;
	}

	m_traceLogger.Uninitialize();
}

HRESULT BigDriveDataObject::CreateInstance(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, void** ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	*ppv = nullptr;

	BigDriveDataObject* pBigDriveDataObject = new BigDriveDataObject(pFolder, cidl, apidl);
	if (!pBigDriveDataObject)
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hr = pBigDriveDataObject->QueryInterface(IID_IDataObject, ppv);
	if (FAILED(hr))
	{
		goto End;
	}

End:

	if (pBigDriveDataObject)
	{
		pBigDriveDataObject->Release();
		pBigDriveDataObject = nullptr;
	}

	return hr;
}

/// <summary>
/// Creates a Shell ID List structure for the selected items.
/// </summary>
/// <param name="pmedium">Storage medium to receive the data.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT BigDriveDataObject::CreateShellIDList(STGMEDIUM* pmedium)
{
	if (!m_pFolder || !m_apidl || m_cidl == 0)
		return E_FAIL;

	// Get the parent folder's absolute PIDL
	PIDLIST_ABSOLUTE pidlFolder = nullptr;
	HRESULT hr = m_pFolder->GetCurFolder(&pidlFolder);
	if (FAILED(hr) || !pidlFolder)
		return hr;

	// Calculate the size needed for the CIDA structure
	// CIDA = header + parent PIDL offset + item PIDL offsets
	size_t cidaHeaderSize = sizeof(UINT) * (2 + m_cidl);

	// Get the size of the parent folder PIDL
	size_t pidlFolderSize = ILGetSize(pidlFolder);

	// Calculate the total size for all item PIDLs
	size_t pidlTotalSize = pidlFolderSize;
	for (UINT i = 0; i < m_cidl; i++)
	{
		pidlTotalSize += ILGetSize(m_apidl[i]);
	}

	// Total size = CIDA header + all PIDLs
	size_t totalSize = cidaHeaderSize + pidlTotalSize;

	// Allocate memory for the CIDA structure
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, totalSize);
	if (!hMem)
	{
		ILFree(pidlFolder);
		return E_OUTOFMEMORY;
	}

	// Lock the global memory to get a pointer to the CIDA structure
	LPVOID lpMem = GlobalLock(hMem);
	if (!lpMem)
	{
		GlobalFree(hMem);
		ILFree(pidlFolder);
		return E_OUTOFMEMORY;
	}

	// Set up the CIDA structure header
	LPBYTE lpCurrent = (LPBYTE)lpMem;
	CIDA* pcida = (CIDA*)lpCurrent;
	pcida->cidl = m_cidl;  // Number of child PIDLs

	// Set the offset to the parent folder PIDL
	lpCurrent = (LPBYTE)lpMem + sizeof(UINT) * (2 + m_cidl); // Skip header and all offsets
	pcida->aoffset[0] = (UINT)(lpCurrent - (LPBYTE)lpMem);

	// Copy the parent folder PIDL
	CopyMemory(lpCurrent, pidlFolder, pidlFolderSize);
	lpCurrent += pidlFolderSize;

	// Copy each child PIDL and set its offset in the CIDA structure
	for (UINT i = 0; i < m_cidl; i++)
	{
		pcida->aoffset[i + 1] = (UINT)(lpCurrent - (LPBYTE)lpMem);
		size_t pidlSize = ILGetSize(m_apidl[i]);
		CopyMemory(lpCurrent, m_apidl[i], pidlSize);
		lpCurrent += pidlSize;
	}

	// Unlock the global memory
	GlobalUnlock(hMem);

	// Set up the storage medium
	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = hMem;
	pmedium->pUnkForRelease = nullptr;

	// Clean up
	ILFree(pidlFolder);

	return S_OK;
}

/// <summary>
/// Creates a file descriptor structure for the selected items.
/// </summary>
/// <param name="pmedium">Storage medium to receive the data.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT BigDriveDataObject::CreateFileDescriptor(STGMEDIUM* pmedium)
{
	if (!m_pFolder || !m_apidl || m_cidl == 0)
		return E_FAIL;

	// Calculate size needed for the FILEGROUPDESCRIPTOR structure
	// FILEGROUPDESCRIPTOR = header + array of FILEDESCRIPTOR structures
	size_t size = sizeof(FILEGROUPDESCRIPTOR) + (m_cidl - 1) * sizeof(FILEDESCRIPTOR);

	// Allocate memory for the FILEGROUPDESCRIPTOR structure
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size);
	if (!hMem)
		return E_OUTOFMEMORY;

	// Lock the global memory to get a pointer to the FILEGROUPDESCRIPTOR structure
	LPVOID lpMem = GlobalLock(hMem);
	if (!lpMem)
	{
		GlobalFree(hMem);
		return E_OUTOFMEMORY;
	}

	// Set up the FILEGROUPDESCRIPTOR structure
	FILEGROUPDESCRIPTOR* pfgd = (FILEGROUPDESCRIPTOR*)lpMem;
	pfgd->cItems = m_cidl;

	// Fill in each FILEDESCRIPTOR structure
	for (UINT i = 0; i < m_cidl; i++)
	{
		FILEDESCRIPTOR* pfd = &pfgd->fgd[i];

		// Initialize the structure
		ZeroMemory(pfd, sizeof(FILEDESCRIPTOR));
		pfd->dwFlags = FD_ATTRIBUTES | FD_FILESIZE;

		// Get the item's name
		STRRET strret;
		ZeroMemory(&strret, sizeof(STRRET));
		HRESULT hr = BigDriveShellFolder::GetBigDriveItemNameFromPidl(m_apidl[i], &strret);
		if (SUCCEEDED(hr))
		{
			// Convert STRRET to a wide string
			WCHAR szName[MAX_PATH];
			hr = StrRetToBufW(&strret, m_apidl[i], szName, ARRAYSIZE(szName));
			if (SUCCEEDED(hr))
			{
				// Copy the name to the FILEDESCRIPTOR
				wcsncpy_s(pfd->cFileName, szName, ARRAYSIZE(pfd->cFileName) - 1);
			}
		}

		// Determine if this is a folder
		const BIGDRIVE_ITEMID* pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(m_apidl[i]);
		if (pItem && pItem->uType == BigDriveItemType_Folder)
		{
			pfd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		}
		else
		{
			pfd->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		}
	}

	// Unlock the global memory
	GlobalUnlock(hMem);

	// Set up the storage medium
	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = hMem;
	pmedium->pUnkForRelease = nullptr;

	return S_OK;
}

HRESULT BigDriveDataObject::CreateDropDescription(STGMEDIUM* pmedium)
{
	HRESULT hr = S_OK;
	DROPDESCRIPTION* pDropDesc = nullptr;

	// Create and populate a DROPDESCRIPTION structure
	HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DROPDESCRIPTION));
	if (!hGlobal)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	pDropDesc = (DROPDESCRIPTION*)::GlobalLock(hGlobal);
	if (!pDropDesc)
	{
		::GlobalFree(hGlobal);
		hGlobal = nullptr;
		hr = E_OUTOFMEMORY;
		goto End;
	}

	// Set the drop description type
	pDropDesc->type = DROPIMAGE_COPY; // or DROPIMAGE_MOVE, etc. depending on your needs

	// Set the message (max 63 chars)
	::wcscpy_s(pDropDesc->szMessage, 64, L"Copy to %1");

	// You can also set a custom insert string if needed
	::wcscpy_s(pDropDesc->szInsert, 260, L"Big Drive");

	::GlobalUnlock(hGlobal);

	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = hGlobal;
	pmedium->pUnkForRelease = nullptr;

End:

	return hr;
}

/// <inheritdoc />
HRESULT BigDriveDataObject::CreateFileContents(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
	HRESULT hr = S_OK;
	BYTE* pData = nullptr;
	SIZE_T dataSize = 0;
	HGLOBAL hGlobal = nullptr;
	void* pDest = nullptr;

	LONG fileIndex = pformatetc->lindex;
	if (fileIndex < 0 || (UINT)fileIndex >= m_cidl)
	{
		hr = DV_E_LINDEX;
		goto End;
	}

	hr = GetFileDataFromPidl(m_apidl[fileIndex], &pData, dataSize);
	if (FAILED(hr) || pData == nullptr)
	{
		goto End;
	}

	hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, dataSize);
	if (!hGlobal)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	pDest = ::GlobalLock(hGlobal);
	if (!pDest)
	{
		::GlobalFree(hGlobal);
		hr = E_OUTOFMEMORY;
		goto End;
	}

	::memcpy(pDest, pData, dataSize);
	::GlobalUnlock(hGlobal);

	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = hGlobal;
	pmedium->pUnkForRelease = nullptr;

End:

	if (pData != nullptr)
	{
		::CoTaskMemFree(pData);
		pData = nullptr;
	}

	return hr;
}

/// <inheritdoc />
HRESULT BigDriveDataObject::CreateFileNameW(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
	HRESULT hr = S_OK;
	STRRET strret = { 0 };
	WCHAR szName[MAX_PATH] = { 0 };
	size_t filenameLen = 0;
	size_t totalSize = 0;
	HGLOBAL hGlobal = nullptr;
	LPWSTR pDest = nullptr;

	// We currently only support one file at a time for simplicity
	if (m_cidl != 1)
	{
		hr = E_NOTIMPL;
		goto End;
	}

	// Get the display name of the first item
	hr = m_pFolder->GetDisplayNameOf(m_apidl[0], SHGDN_FORPARSING, &strret);
	if (FAILED(hr))
	{
		goto End;
	}

	// Convert STRRET to a wide string
	hr = ::StrRetToBufW(&strret, m_apidl[0], szName, ARRAYSIZE(szName));
	if (FAILED(hr))
	{
		goto End;
	}

	// Calculate required memory size
	filenameLen = ::wcslen(szName);
	totalSize = (filenameLen + 1) * sizeof(WCHAR); // +1 for null terminator

	// Allocate global memory for the filename
	hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, totalSize);
	if (!hGlobal)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	// Lock the memory and copy the filename
	pDest = static_cast<LPWSTR>(::GlobalLock(hGlobal));
	if (!pDest)
	{
		::GlobalFree(hGlobal);
		hr = E_OUTOFMEMORY;
		goto End;
	}

	::wcscpy_s(pDest, filenameLen + 1, szName);
	::GlobalUnlock(hGlobal);

	// Set up the storage medium
	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = hGlobal;
	pmedium->pUnkForRelease = nullptr;

End:

	return hr;
}

/// <inheritdoc />
HRESULT BigDriveDataObject::GetFileDataFromPidl(PCUITEMID_CHILD pidl, BYTE** ppData, SIZE_T& dataSize)
{
	HRESULT hr = S_OK;
	STRRET strret = { 0 };
	DriveConfiguration driveConfiguration;
	BigDriveInterfaceProvider* pInterfaceProvider = nullptr;
	IBigDriveFileData* pBigDriveFileData = nullptr;
	BSTR bstrPath = nullptr;
	PIDLIST_ABSOLUTE pidlAbsolute = nullptr;
	IStream* pStream = nullptr;
	LARGE_INTEGER liZero = {0};
	ULARGE_INTEGER uliSize = {};
	ULONG bytesRead = 0;
	PIDLIST_ABSOLUTE pidlFolder = nullptr;
	IStream* pValidatedStream = nullptr;

	if (m_pFolder == nullptr || pidl == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	// Get the display name
	hr = m_pFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &strret);
	if (FAILED(hr))
	{
		goto End;
	}

	hr = BigDriveConfigurationClient::GetDriveConfiguration(m_driveGuid, driveConfiguration);
	if (FAILED(hr))
	{
		goto End;
	}

	pInterfaceProvider = new BigDriveInterfaceProvider(driveConfiguration);
	if (pInterfaceProvider == nullptr)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	hr = pInterfaceProvider->GetIBigDriveFileData(&pBigDriveFileData);
	switch (hr)
	{
	case S_OK:
		break;
	case S_FALSE:
		// Interface isn't Implemented By The Provider
		goto End;
	default:
		break;
	}

	if (!pBigDriveFileData)
	{
		goto End;
	}

	hr = m_pFolder->GetPidlAbsolute(pidlFolder);
	if (FAILED(hr))
	{
		goto End;
	}

	pidlAbsolute = ::ILCombine(pidlFolder, pidl);
	if (pidlAbsolute == nullptr)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	hr = m_pFolder->GetPathForProviders(pidlAbsolute, bstrPath);
	if (FAILED(hr))
	{
		goto End;
	}

	hr = pBigDriveFileData->GetFileData(m_driveGuid, bstrPath, &pStream);
	if (FAILED(hr) || !pStream)
	{
		goto End;
	}

	// Verify pStream is a valid COM object by querying for IStream
	hr = pStream->QueryInterface(IID_IStream, (void**)&pValidatedStream);
	if (FAILED(hr) || !pValidatedStream)
	{
		// Invalid stream object
		if (pStream)
		{
			pStream->Release();
			pStream = nullptr;
		}
		hr = E_INVALIDARG;
		goto End;
	}


	// Seek to the end to get the size
	hr = pValidatedStream->Seek(liZero, STREAM_SEEK_END, &uliSize);
	if (FAILED(hr))
	{
		goto End;
	}

	// Seek back to the beginning
	hr = pValidatedStream->Seek(liZero, STREAM_SEEK_SET, nullptr);
	if (FAILED(hr))
	{
		goto End;
	}

	// Allocate buffer for the data
	dataSize = static_cast<SIZE_T>(uliSize.QuadPart);
	if (dataSize == 0)
	{
		hr = S_FALSE;
		goto End;
	}

	*ppData = (BYTE*)::CoTaskMemAlloc(dataSize);
	if (!*ppData)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	hr = pValidatedStream->Read(*ppData, static_cast<ULONG>(dataSize), &bytesRead);
	if (FAILED(hr) || bytesRead != dataSize)
	{
		// Read failed or didn't read the expected amount of data
		// Clean up the outputs so we don't leak memory

		::CoTaskMemFree(*ppData);
		*ppData = nullptr;
		dataSize = 0;

		goto End;
	}

End:

	if (pValidatedStream)
	{
		pValidatedStream->Release();
		pValidatedStream = nullptr;
	}

	if (pidlFolder)
	{
		::ILFree(pidlFolder);
		pidlFolder = nullptr;
	}

	if (pStream)
	{
		pStream->Release();
		pStream = nullptr;
	}

	if (pidlAbsolute)
	{
		::ILFree(pidlAbsolute);
		pidlAbsolute = nullptr;
	}

	if (bstrPath)
	{
		::SysFreeString(bstrPath);
		bstrPath = nullptr;
	}

	if (pInterfaceProvider)
	{
		delete pInterfaceProvider;
		pInterfaceProvider = nullptr;
	}

	if (pBigDriveFileData)
	{
		pBigDriveFileData->Release();
		pBigDriveFileData = nullptr;
	}

	return hr;

}

