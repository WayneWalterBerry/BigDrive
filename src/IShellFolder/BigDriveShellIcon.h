// <copyright file="BigDriveShellIcon.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// For IShellFolder and related interfaces
#include <shlobj.h> 

#include "BigDriveShellFolder.h"

class BigDriveShellIcon : public
	IExtractIconW,
	IExtractIconA
{
private:

	/// <summary>
	/// Reference count for the COM object.
	/// </summary>
	LONG m_refCount;

	/// <summary>
	/// Stores the drive GUID
	/// </summary>
	CLSID m_driveGuid;

	/// <summary>
	/// Pointer to the parent shell folder
	/// </summary>
	BigDriveShellFolder* m_pParentFolder;

	/// <summary>
	/// Count of items (number of PIDLs)
	/// </summary>
	UINT m_cidl;

	/// <summary>
	/// Array of item ID list pointers (PIDLs)
	/// </summary>
	PCUITEMID_CHILD* m_apidl;

	/// <summary>
	/// Logger that captures trace information for the shell folder.
	/// </summary>
	BigDriveShellFolderTraceLogger m_traceLogger;

private:

	/// <summary>
	/// Private constructor - use CreateInstance to create instances.
	/// </summary>
	/// <param name="pFolder">The parent BigDriveShellFolder object.</param>
	/// <param name="cidl">Count of item IDs in the array.</param>
	/// <param name="apidl">Array of item IDs for which to create the context menu.</param>
	BigDriveShellIcon(const CLSID& driveGuid, BigDriveShellFolder* pParentFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl);

	/// <summary>
	/// Destructor.
	/// </summary>
	~BigDriveShellIcon();

public:



	/// <summary>
	/// Factory method to create an instance of BigDriveShellIcon.
	/// </summary>
	/// <param name="driveGuid">The GUID of the drive.</param>
	/// <param name="pFolder">The parent shell folder.</param>
	/// <param name="cidl">Count of item IDs in the array.</param>
	/// <param name="apidl">Array of item IDs for which to create the icon handler.</param>
	/// <param name="riid">The requested interface ID.</param>
	/// <param name="ppv">On success, receives the requested interface pointer.</param>
	/// <returns>S_OK if successful, or an error code.</returns>
	static HRESULT CreateInstance(const CLSID& driveGuid, BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, void** ppv);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// IUnknown methods

	/// <summary>
	/// Queries the object for a pointer to one of its supported interfaces.
	/// </summary>
	/// <param name="riid">The identifier of the interface being requested.</param>
	/// <param name="ppvObject">A pointer to the interface pointer to be populated.</param>
	/// <returns>
	/// S_OK if the interface is supported; E_NOINTERFACE if not.
	/// </returns>
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

	/// <summary>
	/// Increments the reference count for the object.
	/// </summary>
	/// <returns>The new reference count.</returns>
	ULONG __stdcall AddRef() override;

	/// <summary>
	/// Decrements the reference count for the object. Deletes the object if the reference count reaches zero.
	/// </summary>
	/// <returns>The new reference count.</returns>
	ULONG __stdcall Release() override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// IExtractIconW methods

	/// <summary>
	/// Retrieves the location and index of the icon for a specified item in the BigDrive shell namespace.
	/// The Shell calls this method to determine which icon to display for a given item (file or folder).
	/// 
	/// <para><b>Parameters:</b></para>
	/// <param name="uFlags">[in] Flags specifying icon retrieval options (GIL_*).</param>
	/// <param name="pszFile">[out] Buffer to receive the icon location (DLL or EXE path, or special string).</param>
	/// <param name="cchMax">[in] Size of the pszFile buffer, in characters.</param>
	/// <param name="pIndex">[out] Receives the icon index within the file specified by pszFile.</param>
	/// <param name="pwFlags">[in, out] On input, specifies icon retrieval flags (GIL_*). On output, can specify additional flags.</param>
	/// 
	/// <para><b>Return Value:</b></para>
	/// <returns>
	///   S_OK if the icon location and index were successfully retrieved.<br/>
	///   S_FALSE if a default icon should be used.<br/>
	///   E_FAIL or other COM error codes on failure.
	/// </returns>
	/// 
	/// <para><b>Notes:</b></para>
	/// <list type="bullet">
	///   <item>Set *pszFile to the path of the icon file (e.g., system DLL or EXE).</item>
	///   <item>Set *pIndex to the icon index within the file.</item>
	///   <item>Set *pwFlags to GIL_PERINSTANCE if the icon is per-instance, or GIL_NOTFILENAME if pszFile is not a file path.</item>
	///   <item>Return S_FALSE to let the Shell use the default icon.</item>
	/// </list>
	/// </summary>
	HRESULT __stdcall GetIconLocation(
		UINT uFlags,
		LPWSTR pszFile,
		UINT cchMax,
		int* pIndex,
		UINT* pwFlags);

	/// <summary>
	/// Extracts the icon image for a specified item in the BigDrive shell namespace.
	/// The Shell calls this method if GetIconLocation returns S_OK and expects the actual icon handle.
	/// 
	/// <para><b>Parameters:</b></para>
	/// <param name="pszFile">[in] The icon location string returned by GetIconLocation.</param>
	/// <param name="nIconIndex">[in] The icon index within the file.</param>
	/// <param name="phiconLarge">[out] Receives the large icon handle (32x32).</param>
	/// <param name="phiconSmall">[out] Receives the small icon handle (16x16).</param>
	/// <param name="nIconSize">[in] Specifies the desired icon sizes (LOWORD = large, HIWORD = small).</param>
	/// 
	/// <para><b>Return Value:</b></para>
	/// <returns>
	///   S_OK if the icon(s) were successfully extracted.<br/>
	///   S_FALSE if the Shell should extract the icon itself.<br/>
	///   E_FAIL or other COM error codes on failure.
	/// </returns>
	/// 
	/// <para><b>Notes:</b></para>
	/// <list type="bullet">
	///   <item>If you do not provide the icon, return S_FALSE to let the Shell extract it using the location and index.</item>
	///   <item>If you return icon handles, the Shell will destroy them when done.</item>
	/// </list>
	/// </summary>
	HRESULT __stdcall Extract(
		LPCWSTR pszFile,
		UINT nIconIndex,
		HICON* phiconLarge,
		HICON* phiconSmall,
		UINT nIconSize);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// IExtractIconA methods

	/// <summary>
	/// Retrieves the location and index of the icon for a specified item in the BigDrive shell namespace (ANSI version).
	/// The Shell calls this method to determine which icon to display for a given item (file or folder).
	/// 
	/// <para><b>Parameters:</b></para>
	/// <param name="uFlags">[in] Flags specifying icon retrieval options (GIL_*).</param>
	/// <param name="pszFile">[out] Buffer to receive the icon location (DLL or EXE path, or special string, ANSI).</param>
	/// <param name="cchMax">[in] Size of the pszFile buffer, in characters.</param>
	/// <param name="pIndex">[out] Receives the icon index within the file specified by pszFile.</param>
	/// <param name="pwFlags">[in, out] On input, specifies icon retrieval flags (GIL_*). On output, can specify additional flags.</param>
	/// 
	/// <para><b>Return Value:</b></para>
	/// <returns>
	///   S_OK if the icon location and index were successfully retrieved.<br/>
	///   S_FALSE if a default icon should be used.<br/>
	///   E_FAIL or other COM error codes on failure.
	/// </returns>
	/// 
	/// <para><b>Notes:</b></para>
	/// <list type="bullet">
	///   <item>Set *pszFile to the path of the icon file (e.g., system DLL or EXE).</item>
	///   <item>Set *pIndex to the icon index within the file.</item>
	///   <item>Set *pwFlags to GIL_PERINSTANCE if the icon is per-instance, or GIL_NOTFILENAME if pszFile is not a file path.</item>
	///   <item>Return S_FALSE to let the Shell use the default icon.</item>
	/// </list>
	/// </summary>
	HRESULT __stdcall GetIconLocation(
		UINT uFlags,
		LPSTR pszFile,
		UINT cchMax,
		int* pIndex,
		UINT* pwFlags);

	/// <summary>
	/// Extracts the icon image for a specified item in the BigDrive shell namespace (ANSI version).
	/// The Shell calls this method if GetIconLocation returns S_OK and expects the actual icon handle.
	/// 
	/// <para><b>Parameters:</b></para>
	/// <param name="pszFile">[in] The icon location string returned by GetIconLocation (ANSI).</param>
	/// <param name="nIconIndex">[in] The icon index within the file.</param>
	/// <param name="phiconLarge">[out] Receives the large icon handle (32x32).</param>
	/// <param name="phiconSmall">[out] Receives the small icon handle (16x16).</param>
	/// <param name="nIconSize">[in] Specifies the desired icon sizes (LOWORD = large, HIWORD = small).</param>
	/// 
	/// <para><b>Return Value:</b></para>
	/// <returns>
	///   S_OK if the icon(s) were successfully extracted.<br/>
	///   S_FALSE if the Shell should extract the icon itself.<br/>
	///   E_FAIL or other COM error codes on failure.
	/// </returns>
	/// 
	/// <para><b>Notes:</b></para>
	/// <list type="bullet">
	///   <item>If you do not provide the icon, return S_FALSE to let the Shell extract it using the location and index.</item>
	///   <item>If you return icon handles, the Shell will destroy them when done.</item>
	/// </list>
	/// </summary>
	HRESULT __stdcall Extract(
		LPCSTR pszFile,
		UINT nIconIndex,
		HICON* phiconLarge,
		HICON* phiconSmall,
		UINT nIconSize);
};