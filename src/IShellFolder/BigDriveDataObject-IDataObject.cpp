// <copyright file="BigDriveDataObject-IDataObject.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveDataObject.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <shlobj.h>
#include <shlwapi.h>

// Common clipboard formats used for shell operations
#ifndef CFSTR_SHELLIDLIST
#define CFSTR_SHELLIDLIST TEXT("Shell IDList Array")
#endif

#ifndef CFSTR_FILEDESCRIPTOR
#define CFSTR_FILEDESCRIPTOR TEXT("FileGroupDescriptor")
#endif

/// <summary>
/// Retrieves data from the data object in the specified format.
/// </summary>
/// <param name="pformatetc">Format information requested by the caller.</param>
/// <param name="pmedium">Storage medium to receive the data.</param>
/// <returns>S_OK if successful; otherwise, an error code.</returns>
HRESULT __stdcall BigDriveDataObject::GetData(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
    if (!pformatetc || !pmedium)
        return E_INVALIDARG;

    // Initialize the medium structure
    ZeroMemory(pmedium, sizeof(STGMEDIUM));

    // Handle Shell IDList Array - used for shell drag-drop operations
    if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_SHELLIDLIST) &&
        pformatetc->tymed & TYMED_HGLOBAL)
    {
        return CreateShellIDList(pmedium);
    }
    // Handle File Descriptor - provides metadata about files being dragged
    else if (pformatetc->cfFormat == RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR) &&
        pformatetc->tymed & TYMED_HGLOBAL)
    {
        return CreateFileDescriptor(pmedium);
    }
    // Handle CF_HDROP - classic file drop format
    else if (pformatetc->cfFormat == CF_HDROP &&
        pformatetc->tymed & TYMED_HGLOBAL)
    {
        // Not implemented - would require converting BigDrive paths to local file paths
        // This is only needed if drag-dropping to applications that don't use Shell IDList
        return DV_E_FORMATETC;
    }

    return DV_E_FORMATETC;
}

/// <summary>
/// Retrieves data and writes it to the specified medium. For BigDriveDataObject,
/// this method is not fully implemented as it's rarely used in typical shell operations.
/// </summary>
/// <param name="pformatetc">Format information requested by the caller.</param>
/// <param name="pmedium">Storage medium where the data should be written.</param>
/// <returns>E_NOTIMPL since this is not implemented.</returns>
HRESULT __stdcall BigDriveDataObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
    return E_NOTIMPL;
}

/// <summary>
/// Checks if data in the specified format is available.
/// </summary>
/// <param name="pformatetc">Format information to check.</param>
/// <returns>S_OK if the format is supported; otherwise, a DV_E_* error code.</returns>
HRESULT __stdcall BigDriveDataObject::QueryGetData(FORMATETC* pformatetc)
{
    if (!pformatetc)
        return E_INVALIDARG;

    // Check if the requested aspect is supported (we only support DVASPECT_CONTENT)
    if (pformatetc->dwAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    // Check clipboard formats we support
    CLIPFORMAT cf = pformatetc->cfFormat;
    if ((cf == RegisterClipboardFormat(CFSTR_SHELLIDLIST) &&
        (pformatetc->tymed & TYMED_HGLOBAL)) ||
        (cf == RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR) &&
            (pformatetc->tymed & TYMED_HGLOBAL)) ||
        (cf == CF_HDROP &&
            (pformatetc->tymed & TYMED_HGLOBAL)))
    {
        return S_OK;
    }

    return DV_E_FORMATETC;
}

/// <summary>
/// Gets a canonical format structure.
/// </summary>
/// <param name="pformatectIn">The format to canonicalize.</param>
/// <param name="pformatetcOut">Receives the canonicalized format.</param>
/// <returns>E_NOTIMPL since identical format structures are used.</returns>
HRESULT __stdcall BigDriveDataObject::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut)
{
    if (!pformatetcOut)
        return E_INVALIDARG;

    // Initialize the output format
    ZeroMemory(pformatetcOut, sizeof(FORMATETC));

    // According to COM spec, for data objects that don't provide a difference
    // between source and target devices, return DATA_S_SAMEFORMATETC
    return DATA_S_SAMEFORMATETC;
}

/// <summary>
/// Sets data in the data object. This is only required for clipboard operations
/// and is not needed for simple drag-drop operations.
/// </summary>
/// <param name="pformatetc">Format information.</param>
/// <param name="pmedium">Data to set.</param>
/// <param name="fRelease">TRUE if the data object should release the medium.</param>
/// <returns>E_NOTIMPL since setting data is not supported.</returns>
HRESULT __stdcall BigDriveDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
    return E_NOTIMPL;
}

/// <summary>
/// Creates an enumerator for the formats supported by this data object.
/// </summary>
/// <param name="dwDirection">Direction of enumeration (DATADIR_GET or DATADIR_SET).</param>
/// <param name="ppenumFormatEtc">Receives the enumerator pointer.</param>
/// <returns>E_NOTIMPL since format enumeration is not implemented.</returns>
HRESULT __stdcall BigDriveDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
    // For a full implementation, you would create an IEnumFORMATETC object
    // that enumerates all supported formats
    return E_NOTIMPL;
}

/// <summary>
/// Sets up an advisory connection to the data object.
/// </summary>
/// <param name="pformatetc">Format information.</param>
/// <param name="advf">Advisory flags.</param>
/// <param name="pAdvSink">Advisory sink.</param>
/// <param name="pdwConnection">Receives the connection token.</param>
/// <returns>E_NOTIMPL since advisory connections are not supported.</returns>
HRESULT __stdcall BigDriveDataObject::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

/// <summary>
/// Removes an advisory connection.
/// </summary>
/// <param name="dwConnection">The connection token.</param>
/// <returns>E_NOTIMPL since advisory connections are not supported.</returns>
HRESULT __stdcall BigDriveDataObject::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

/// <summary>
/// Creates an enumerator for the advisory connections.
/// </summary>
/// <param name="ppenumAdvise">Receives the enumerator pointer.</param>
/// <returns>E_NOTIMPL since advisory connections are not supported.</returns>
HRESULT __stdcall BigDriveDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
