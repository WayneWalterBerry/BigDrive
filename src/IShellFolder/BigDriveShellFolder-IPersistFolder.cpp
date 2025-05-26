// <copyright file="BigDriveShellFolder-IPersistFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>

#include "pch.h"

#include "BigDriveShellFolder.h"

#include "BigDriveShellFolderTraceLogger.h"

#include <shlobj.h>
#include <objbase.h>

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

    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__);

    if (!pidl)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    // Free any existing PIDL
    if (m_pidlAbsolute)
    {
        ::ILFree(const_cast<LPITEMIDLIST>(m_pidlAbsolute));
        m_pidlAbsolute = nullptr;
    }

    // Clone and store the new PIDL
    m_pidlAbsolute = ILClone(pidl);
    if (!m_pidlAbsolute)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

End:

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}