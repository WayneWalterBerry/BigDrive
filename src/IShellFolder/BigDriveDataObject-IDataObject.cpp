// <copyright file="BigDriveDataObject-IDataObject.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveDataObject.h"

#include "Logging\BigDriveShellFolderTraceLogger.h"
#include "..\BigDrive.Client\DriveConfiguration.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "..\BigDrive.Client\Interfaces\IBigDriveFileOperations.h"

#include <shlobj.h>
#include <shlwapi.h>

#ifndef CFSTR_SHELLIDLIST
#define CFSTR_SHELLIDLIST TEXT("Shell IDList Array")
#endif

#ifndef CFSTR_FILEDESCRIPTOR
#define CFSTR_FILEDESCRIPTOR TEXT("FileGroupDescriptor")
#endif

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::GetData(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
	/// <inheritdoc />
	HRESULT hr = S_OK;
	BSTR bstrTargetFolder = nullptr;
	BYTE* pData = nullptr;
	SIZE_T dataSize = 0;

	if (pformatetc == nullptr || pmedium == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	::ZeroMemory(pmedium, sizeof(STGMEDIUM));

	if (pformatetc->cfFormat == ::RegisterClipboardFormat(CFSTR_SHELLIDLIST) && (pformatetc->tymed & TYMED_HGLOBAL))
	{
		hr = CreateShellIDList(pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR) && (pformatetc->tymed & TYMED_HGLOBAL))
	{
		hr = CreateFileDescriptor(pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == CF_HDROP && (pformatetc->tymed & TYMED_HGLOBAL))
	{
		hr = DV_E_FORMATETC;
		goto End;
	}
	else if (pformatetc->cfFormat == ::RegisterClipboardFormat(CFSTR_FILECONTENTS) && (pformatetc->tymed & TYMED_HGLOBAL))
	{
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

		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, dataSize);
		if (!hGlobal)
		{
			hr = E_OUTOFMEMORY;
			goto End;
		}

		void* pDest = ::GlobalLock(hGlobal);
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

		hr = S_OK;
		goto End;
	}
	else
	{
		hr = DV_E_FORMATETC;
		goto End;
	}

End:

	if (pData != nullptr)
	{
		::CoTaskMemFree(pData);
		pData = nullptr;
	}

	if (bstrTargetFolder)
	{
		::SysFreeString(bstrTargetFolder);
		bstrTargetFolder = nullptr;
	}

	if (FAILED(hr) && pmedium != nullptr)
	{
		::ZeroMemory(pmedium, sizeof(STGMEDIUM));
	}

	return hr;

}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
	HRESULT hr = E_NOTIMPL;
	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::QueryGetData(FORMATETC* pformatetc)
{
	HRESULT hr = S_OK;
	CLIPFORMAT cf = 0;

	m_traceLogger.LogEnter(__FUNCTION__, *pformatetc);

	if (pformatetc == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	if (pformatetc->dwAspect != DVASPECT_CONTENT)
	{
		hr = DV_E_DVASPECT;
		goto End;
	}

	cf = pformatetc->cfFormat;

	if ((cf == ::RegisterClipboardFormat(CFSTR_SHELLIDLIST) && (pformatetc->tymed & TYMED_HGLOBAL)) ||
		(cf == ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR) && (pformatetc->tymed & TYMED_HGLOBAL)) ||
		(cf == ::RegisterClipboardFormat(CFSTR_FILECONTENTS) && (pformatetc->tymed & TYMED_HGLOBAL)))
	{
		hr = S_OK;
		goto End;
	}

	hr = DV_E_FORMATETC;

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut)
{
	HRESULT hr = S_OK;

	if (pformatetcOut == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	::ZeroMemory(pformatetcOut, sizeof(FORMATETC));
	hr = DATA_S_SAMEFORMATETC;

End:

	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
	/// <inheritdoc />
	HRESULT hr = E_NOTIMPL;
	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	HRESULT hr = S_OK;

	m_traceLogger.LogEnter(__FUNCTION__);

	if (ppenumFormatEtc == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	*ppenumFormatEtc = nullptr;

	if (dwDirection != DATADIR_GET)
	{
		hr = E_NOTIMPL;
		goto End;
	}

	// Define the supported formats
	static const FORMATETC formats[] = {
		{
			(CLIPFORMAT)::RegisterClipboardFormat(CFSTR_SHELLIDLIST), // cfFormat
			nullptr,                                                  // ptd
			DVASPECT_CONTENT,                                         // dwAspect
			-1,                                                       // lindex
			TYMED_HGLOBAL                                             // tymed
		},
		{
			(CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR),
			nullptr,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		},
		{
			(CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILECONTENTS),
			nullptr,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		}
	};

	hr = ::SHCreateStdEnumFmtEtc(sizeof(formats) / sizeof(FORMATETC), formats, ppenumFormatEtc);
	if (FAILED(hr))
	{
		goto End;
	}

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	/// <inheritdoc />
	HRESULT hr = OLE_E_ADVISENOTSUPPORTED;
	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::DUnadvise(DWORD dwConnection)
{
	/// <inheritdoc />
	HRESULT hr = OLE_E_ADVISENOTSUPPORTED;
	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
	/// <inheritdoc />
	HRESULT hr = OLE_E_ADVISENOTSUPPORTED;
	return hr;
}
