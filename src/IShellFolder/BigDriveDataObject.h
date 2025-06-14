// <copyright file="BigDriveDataObject.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "BigDriveShellFolder.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <shlobj.h>

/// <summary>
/// Implements the IDataObject interface for BigDrive shell folder extensions.
/// Provides data transfer and clipboard support for drag-and-drop and copy-paste operations.
/// </summary>
class BigDriveDataObject : public IDataObject
{
private:

    /// <summary>
    /// Reference count for COM lifetime management.
    /// </summary>
    LONG m_cRef;

    /// <summary>
    /// Pointer to the parent shell folder.
    /// </summary>
    BigDriveShellFolder* m_pFolder;

    /// <summary>
    /// Number of item IDs in the array.
    /// </summary>
    UINT m_cidl;

    /// <summary>
    /// Array of item IDs (PIDLs) representing the selected items.
    /// </summary>
    PCUITEMID_CHILD* m_apidl;

    /// <summary>
    /// Logger that captures trace information for the shell folder.
    /// </summary>
    BigDriveShellFolderTraceLogger m_traceLogger;

    /// <summary>
    /// Specifies the preferred effect as a DWORD value.
    /// </summary>
    DWORD m_dwPreferredEffect;

	/// <summary>
	/// Stores the value representing the performed effect.
	/// </summary>
	DWORD m_dwPerformedEffect;

	/// <summary>
	/// Indicates whether a paste operation was successful.
	/// </summary>
	DWORD m_dwPasteSucceeded;

    /// <summary>
    /// GUID representing the drive associated with the data object.
    /// </summary>
    /// <remarks>
    /// This GUID is used to identify the drive for operations like drag-and-drop
    /// and clipboard actions, ensuring that the correct drive is targeted.
	/// </remarks>
    GUID m_driveGuid;

    /// <summary>
    /// A variable that holds a drop description structure.
    /// </summary>
    DROPDESCRIPTION m_dropDescription;

    /// <summary>
    /// Indicates whether the default drag image should be used.
    /// </summary>
    BOOL m_bUseDefaultDragImage; 

private:

    /// <summary>
    /// Initializes a new instance of the <see cref="BigDriveDataObject"/> class.
    /// </summary>
    /// <param name="pFolder">Pointer to the parent shell folder.</param>
    /// <param name="cidl">Number of item IDs.</param>
    /// <param name="apidl">Array of item IDs.</param>
    BigDriveDataObject(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl);

public:

    /// <summary>
    /// Destroys an instance of the <see cref="BigDriveDataObject"/> class.
    /// </summary>
    ~BigDriveDataObject();

    static HRESULT CreateInstance(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, void** ppv);

    /// <summary>
    /// Queries for a pointer to a supported interface.
    /// </summary>
    /// <param name="riid">The interface identifier.</param>
    /// <param name="ppv">Receives the interface pointer.</param>
    /// <returns>S_OK if successful; otherwise, E_NOINTERFACE.</returns>
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

    /// <summary>
    /// Increments the reference count.
    /// </summary>
    /// <returns>The new reference count.</returns>
    STDMETHODIMP_(ULONG) AddRef();

    /// <summary>
    /// Decrements the reference count and deletes the object if the count reaches zero.
    /// </summary>
    /// <returns>The new reference count.</returns>
    STDMETHODIMP_(ULONG) Release();

    /// <summary>
    /// Retrieves the data described by the specified FORMATETC structure.
    /// </summary>
    /// <param name="pformatetc">Pointer to the FORMATETC structure.</param>
    /// <param name="pmedium">Pointer to the STGMEDIUM structure to receive the data.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP GetData(FORMATETC* pformatetc, STGMEDIUM* pmedium);

    /// <summary>
    /// Places data into the specified storage medium.
    /// </summary>
    /// <param name="pformatetc">Pointer to the FORMATETC structure.</param>
    /// <param name="pmedium">Pointer to the STGMEDIUM structure to receive the data.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium);

    /// <summary>
    /// Determines whether the data object is capable of rendering the data described in the FORMATETC structure.
    /// </summary>
    /// <param name="pformatetc">Pointer to the FORMATETC structure.</param>
    /// <returns>S_OK if the format is supported; otherwise, S_FALSE or an error code.</returns>
    STDMETHODIMP QueryGetData(FORMATETC* pformatetc);

    /// <summary>
    /// Provides a standard FORMATETC structure that is logically equivalent to the one that is specified.
    /// </summary>
    /// <param name="pformatectIn">Pointer to the input FORMATETC structure.</param>
    /// <param name="pformatetcOut">Pointer to the output FORMATETC structure.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut);

    /// <summary>
    /// Transfers data to the object.
    /// </summary>
    /// <param name="pformatetc">Pointer to the FORMATETC structure.</param>
    /// <param name="pmedium">Pointer to the STGMEDIUM structure containing the data.</param>
    /// <param name="fRelease">TRUE if the data object should release the storage medium.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);

    /// <summary>
    /// Creates an object for enumerating the FORMATETC structures for a data object.
    /// </summary>
    /// <param name="dwDirection">The direction of the data (GET or SET).</param>
    /// <param name="ppenumFormatEtc">Receives the enumerator interface pointer.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);

    /// <summary>
    /// Establishes a connection between a data object and an advise sink so that the advise sink can receive notifications.
    /// </summary>
    /// <param name="pformatetc">Pointer to the FORMATETC structure.</param>
    /// <param name="advf">Advise flags.</param>
    /// <param name="pAdvSink">Pointer to the IAdviseSink interface.</param>
    /// <param name="pdwConnection">Receives the connection token.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);

    /// <summary>
    /// Destroys a notification connection that was previously established.
    /// </summary>
    /// <param name="dwConnection">The connection token.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP DUnadvise(DWORD dwConnection);

    /// <summary>
    /// Creates an object that can be used to enumerate the current advisory connections.
    /// </summary>
    /// <param name="ppenumAdvise">Receives the enumerator interface pointer.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

private:

    /// <summary>
    /// Creates a shell ID list in the specified storage medium.
    /// </summary>
    /// <param name="pmedium">Pointer to the STGMEDIUM structure to receive the ID list.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT CreateShellIDList(STGMEDIUM* pmedium);

    /// <summary>
    /// Creates a file descriptor in the specified storage medium.
    /// </summary>
    /// <param name="pmedium">Pointer to the STGMEDIUM structure to receive the file descriptor.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT CreateFileDescriptor(STGMEDIUM* pmedium);

    /// <summary>
    /// Initializes a drop description for a data object using the specified storage medium.
    /// </summary>
    /// <param name="pmedium">A pointer to a STGMEDIUM structure that receives the drop description.</param>
    /// <returns>Returns an HRESULT indicating success or failure of the operation.</returns>
    HRESULT CreateDropDescription(STGMEDIUM* pmedium);

    /// <summary>
    /// Creates file contents in the specified storage medium according to the given format descriptor.
    /// </summary>
    /// <param name="pformatetc">A pointer to a FORMATETC structure that defines the format, medium, and target device for the data.</param>
    /// <param name="pmedium">A pointer to a STGMEDIUM structure that, on successful return, receives the created file contents.</param>
    /// <returns>Returns an HRESULT indicating success or failure of the operation.</returns>
    HRESULT CreateFileContents(FORMATETC* pformatetc, STGMEDIUM* pmedium);

    /// <summary>
    /// Creates filename data in wide character format for drag-and-drop operations.
    /// </summary>
    /// <param name="pformatetc">Format specification for the data.</param>
    /// <param name="pmedium">Storage medium to receive the data.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    HRESULT CreateFileNameW(FORMATETC* pformatetc, STGMEDIUM* pmedium);

    /// <summary>
    /// Creates an HDROP structure containing file paths for shell drag-and-drop operations.
    /// 
    /// This method populates a STGMEDIUM with a global memory block containing a DROPFILES structure,
    /// which holds the absolute file paths of the selected items. The Windows Shell uses this data
    /// format (CF_HDROP) to enable dragging files between applications and folders.
    /// </summary>
    /// <param name="pformatetc">
    /// Pointer to a FORMATETC structure that describes the format, medium, and target device
    /// to use when passing the data. The method expects CF_HDROP as the format.
    /// </param>
    /// <param name="pmedium">
    /// [out] Pointer to a STGMEDIUM structure that receives the HDROP data. On successful return,
    /// contains a global memory handle (TYMED_HGLOBAL) with the DROPFILES structure.
    /// </param>
    /// <returns>
    /// S_OK if successful;
    /// E_INVALIDARG if input parameters are null or invalid;
    /// E_OUTOFMEMORY if memory allocation fails;
    /// Other HRESULT error codes as appropriate.
    /// </returns>
    /// <remarks>
    /// The HDROP structure contains:
    /// - A DROPFILES header with fWide=TRUE to indicate Unicode paths
    /// - A sequence of null-terminated Unicode file paths
    /// - A final null terminator marking the end of the path list
    /// 
    /// The caller takes ownership of the returned STGMEDIUM and is responsible for releasing 
    /// it when no longer needed.
    /// </remarks>
    HRESULT CreateHDrop(FORMATETC* pformatetc, STGMEDIUM* pmedium);

    HRESULT GetFileDataFromPidl(PCUITEMID_CHILD pidl, BYTE** ppData, SIZE_T& dataSize);
};