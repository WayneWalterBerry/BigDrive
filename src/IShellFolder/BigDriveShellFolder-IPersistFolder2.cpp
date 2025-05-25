// <copyright file="BigDriveShellFolder-IPersistFolder2.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>

#include "pch.h"

#include "BigDriveShellFolder.h"
#include "BigDriveShellFolderTraceLogger.h"
#include "LaunchDebugger.h"

#include <shlobj.h>
#include <objbase.h>

/// <summary>
/// Retrieves the current absolute PIDL for this Shell Folder.
/// This is required for IShellFolder extensions to allow the shell to query the folder's location.
/// </summary>
/// <param name="ppidl">Address of a pointer that receives the PIDL. The caller is responsible for freeing it with ILFree.</param>
/// <returns>S_OK if successful; E_POINTER if ppidl is null; E_OUTOFMEMORY if cloning fails.</returns>
HRESULT __stdcall BigDriveShellFolder::GetCurFolder(PIDLIST_ABSOLUTE* ppidl)
{
    HRESULT hr = S_OK;

    ::LaunchDebugger();

    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__, m_pidl);

    if (!ppidl)
    {
        hr = E_POINTER;
        goto End;
    }

    *ppidl = nullptr;

    if (m_pidl)
    {
        *ppidl = ILClone(m_pidl);
        if (!*ppidl)
        {
            hr = E_OUTOFMEMORY;
            goto End;
        }
    }

End:

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}