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
#include "RegisterClipboardFormats.h"

#include <shlobj.h>
#include <shlwapi.h>

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::GetData(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
	HRESULT hr = S_OK;
	BSTR bstrTargetFolder = nullptr;
	BYTE* pData = nullptr;
	SIZE_T dataSize = 0;

	m_traceLogger.LogEnter(__FUNCTION__, *pformatetc);

	if (pformatetc == nullptr || pmedium == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	::ZeroMemory(pmedium, sizeof(STGMEDIUM));

	if (pformatetc->cfFormat == g_cfShellIdList)
	{
		// Verify the medium type is supported
		if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
		{
			hr = DV_E_TYMED;
			goto End;
		}

		// Verify the aspect is content
		if (pformatetc->dwAspect != DVASPECT_CONTENT)
		{
			hr = DV_E_DVASPECT;
			goto End;
		}

		hr = CreateShellIDList(pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == g_cfFileDescriptor)
	{
		// Verify the medium type is supported
		if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
		{
			hr = DV_E_TYMED;
			goto End;
		}

		// Verify the aspect is content
		if (pformatetc->dwAspect != DVASPECT_CONTENT)
		{
			hr = DV_E_DVASPECT;
			goto End;
		}

		hr = CreateFileDescriptor(pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == g_cfDropDescription)
	{
		// Verify the medium type is supported
		if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
		{
			hr = DV_E_TYMED;
			goto End;
		}

		// Verify the aspect is content
		if (pformatetc->dwAspect != DVASPECT_CONTENT)
		{
			hr = DV_E_DVASPECT;
			goto End;
		}

		hr = CreateDropDescription(pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == g_cfFileContents)
	{
		// Verify the medium type is supported
		if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
		{
			hr = DV_E_TYMED;
			goto End;
		}

		// Verify the aspect is content
		if (pformatetc->dwAspect != DVASPECT_CONTENT)
		{
			hr = DV_E_DVASPECT;
			goto End;
		}

		hr = CreateFileContents(pformatetc, pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == g_cfFileNameW)
	{
		// Verify the medium type is supported
		if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
		{
			hr = DV_E_TYMED;
			goto End;
		}

		// Verify the aspect is content
		if (pformatetc->dwAspect != DVASPECT_CONTENT)
		{
			hr = DV_E_DVASPECT;
			goto End;
		}

		hr = CreateFileNameW(pformatetc, pmedium);
		goto End;
	}
	else if (pformatetc->cfFormat == g_cfHDrop)
	{
		// This is not suported in BigDrive, since all the files are virtualized.
		hr = DV_E_FORMATETC;
		goto End;
	}
	else if (pformatetc->cfFormat == g_cfPreferredDropEffect)
	{
		// Verify the medium type is supported
		if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
		{
			hr = DV_E_TYMED;
			goto End;
		}

		// Verify the aspect is content
		if (pformatetc->dwAspect != DVASPECT_CONTENT)
		{
			hr = DV_E_DVASPECT;
			goto End;
		}

		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));
		if (!pmedium->hGlobal)
		{
			hr = E_OUTOFMEMORY;
			goto End;
		}

		DWORD* pdwEffect = static_cast<DWORD*>(::GlobalLock(pmedium->hGlobal));
		if (!pdwEffect)
		{
			hr = E_UNEXPECTED;
			goto End;
		}

		// Use the member variable for the preferred effect
		*pdwEffect = m_dwPreferredEffect; 

		// No need to release anything
		pmedium->pUnkForRelease = nullptr; 

		::GlobalUnlock(pmedium->hGlobal);
		
		goto End;
	}
	else if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT))
	{
		pmedium->tymed = TYMED_HGLOBAL;
		
		pmedium->hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));
		
		if (!pmedium->hGlobal)
		{
			return E_OUTOFMEMORY;
		}

		DWORD* pdwEffect = static_cast<DWORD*>(::GlobalLock(pmedium->hGlobal));
		if (!pdwEffect)
		{
			return E_UNEXPECTED;
		}

		// Use the member variable for the performed effect
		*pdwEffect = m_dwPerformedEffect; 

		// No need to release anything
		pmedium->pUnkForRelease = nullptr; 

		::GlobalUnlock(pmedium->hGlobal);
		
		goto End;
	}
	else if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_PASTESUCCEEDED))
	{
		pmedium->tymed = TYMED_HGLOBAL;

		pmedium->hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));

		if (!pmedium->hGlobal)
		{
			return E_OUTOFMEMORY;
		}

		DWORD* pdwEffect = static_cast<DWORD*>(::GlobalLock(pmedium->hGlobal));
		if (!pdwEffect)
		{
			return E_UNEXPECTED;
		}

		// Use the member variable for the performed effect
		*pdwEffect = m_dwPasteSucceeded;

		// No need to release anything
		pmedium->pUnkForRelease = nullptr;

		::GlobalUnlock(pmedium->hGlobal);

		goto End;
	}
	else
	{
		hr = DV_E_FORMATETC;
		goto End;
	}

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

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

	if ((pformatetc->tymed & TYMED_HGLOBAL) == 0)
	{
		hr = DV_E_TYMED;
		goto End;
	}

	cf = pformatetc->cfFormat;

	if ((cf == g_cfShellIdList) ||
		(cf == g_cfFileDescriptor) ||
		(cf == g_cfFileContents) ||
		(cf == g_cfDropDescription) ||
		(cf == g_cfFileNameW))
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

	m_traceLogger.LogEnter(__FUNCTION__);

	if (pformatetcOut == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	::ZeroMemory(pformatetcOut, sizeof(FORMATETC));
	hr = DATA_S_SAMEFORMATETC;

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

	return hr;
}

/// <inheritdoc />
HRESULT __stdcall BigDriveDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
	HRESULT hr = E_NOTIMPL;

	m_traceLogger.LogEnter(__FUNCTION__, *pformatetc);
	
	if (pmedium == nullptr || pformatetc == nullptr)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	if (pmedium->tymed != TYMED_HGLOBAL || !pmedium->hGlobal)
	{
		hr = DV_E_TYMED;
		goto End;
	}

	// Handle known formats that might be set during drag-drop
	if (pformatetc->cfFormat == g_cfPreferredDropEffect ||
		pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT) ||
		pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_PASTESUCCEEDED))
	{
		DWORD *pdwEffect = static_cast<DWORD*>(::GlobalLock(pmedium->hGlobal));
		if (!pdwEffect)
		{
			hr = E_UNEXPECTED;
			goto End;
		}

		// Store the value in member variables if needed
		if (pformatetc->cfFormat == g_cfPreferredDropEffect)
		{
			m_dwPreferredEffect = *pdwEffect;
		}
		else if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT))
		{
			m_dwPerformedEffect = *pdwEffect;
		}
		else if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_PASTESUCCEEDED))
		{
			m_dwPasteSucceeded = *pdwEffect;
		}

		::GlobalUnlock(pmedium->hGlobal);

		// If fRelease is TRUE, we're now responsible for releasing the medium
		if (fRelease)
		{
			::ReleaseStgMedium(pmedium);
		}

		hr = S_OK;
	}
	else if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_DROPDESCRIPTION))
	{
		DROPDESCRIPTION* pDropDesc = static_cast<DROPDESCRIPTION*>(::GlobalLock(pmedium->hGlobal));
		if (!pDropDesc)
		{
			hr = E_UNEXPECTED;
			goto End;
		}

		// Store the drop description
		::memcpy(&m_dropDescription, pDropDesc, sizeof(DROPDESCRIPTION));

		::GlobalUnlock(pmedium->hGlobal);

		// If fRelease is TRUE, we're now responsible for releasing the medium
		if (fRelease)
		{
			::ReleaseStgMedium(pmedium);
		}

		hr = S_OK;
	}
	else if (pformatetc->cfFormat == RegisterClipboardFormat(L"UsingDefaultDragImage"))
	{
		BOOL* pbUseDefaultDragImage = static_cast<BOOL*>(::GlobalLock(pmedium->hGlobal));
		if (!pbUseDefaultDragImage)
		{
			hr = E_UNEXPECTED;
			goto End;
		}

		// Store whether to use the default drag image
		m_bUseDefaultDragImage = *pbUseDefaultDragImage;

		::GlobalUnlock(pmedium->hGlobal);

		// If fRelease is TRUE, we're now responsible for releasing the medium
		if (fRelease)
		{
			::ReleaseStgMedium(pmedium);
		}

		hr = S_OK;
	}
	else
	{
		// For formats we don't handle, return E_NOTIMPL
		hr = E_NOTIMPL;
	}

End:

	m_traceLogger.LogExit(__FUNCTION__, hr);

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
			g_cfShellIdList, // cfFormat
			nullptr,                                                  // ptd
			DVASPECT_CONTENT,                                         // dwAspect
			-1,                                                       // lindex
			TYMED_HGLOBAL                                             // tymed
		},
		{
			g_cfFileDescriptor,
			nullptr,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		},
		{
			g_cfFileContents,
			nullptr,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		},
		{
			g_cfDropDescription,
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
