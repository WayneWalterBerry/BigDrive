// <copyright file="BigDriveShellFolder-IPersistFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>
// <summary>
//   Implements the IPersistFolder interface for the BigDriveShellFolder class.
//   These methods are essential for IShellFolder extensions, enabling the shell to
//   identify and initialize custom namespace extensions. GetClassID provides the
//   unique CLSID for the folder, while Initialize sets up the folder's location
//   within the shell namespace using a PIDL.
// </summary>

#include "pch.h"

#include "BigDriveShellFolder.h"

#include "BigDriveShellFolderTraceLogger.h"

#include <shlobj.h>
#include <objbase.h>

/// <summary>
/// Retrieves the class identifier (CLSID) for this Shell Folder extension.
/// This allows the Windows Shell to uniquely identify the custom IShellFolder implementation.
/// For namespace extensions, this CLSID is used during registration and binding.
/// </summary>
/// <param name="pClassID">Pointer to a CLSID that receives the class identifier.</param>
/// <returns>S_OK if successful; E_POINTER if pClassID is null.</returns>
HRESULT __stdcall BigDriveShellFolder::GetClassID(CLSID* pClassID)
{
    HRESULT hr = S_OK;

    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__, pClassID);

    if (!pClassID)
    {
        hr = E_POINTER;
        goto End;
    }

    *pClassID = m_driveGuid;

End:

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}

/// <summary>
/// Initializes the Shell Folder extension with its absolute location in the Shell namespace.
/// The Shell calls this method after creating the folder object, passing the absolute PIDL.
/// This enables the extension to know its position in the namespace hierarchy and is required
/// for correct operation of IShellFolder extensions.
/// </summary>
/// <param name="pidl">The absolute PIDL that identifies the folder's location.</param>
/// <returns>S_OK if successful; E_INVALIDARG if pidl is null; E_OUTOFMEMORY if cloning fails.</returns>
HRESULT __stdcall BigDriveShellFolder::Initialize(PCIDLIST_ABSOLUTE pidl)
{
    HRESULT hr = S_OK;

    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__, m_pidl);

    if (!pidl)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    // Free any existing PIDL
    if (m_pidl)
    {
        ::ILFree(const_cast<LPITEMIDLIST>(m_pidl));
        m_pidl = nullptr;
    }

    // Clone and store the new PIDL
    m_pidl = ILClone(pidl);
    if (!m_pidl)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

End:

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, hr);


    return hr;
}