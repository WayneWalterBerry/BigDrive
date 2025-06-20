// <copyright file="BigDriveShellFolder-IPersist.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>

#include "pch.h"

#include "BigDriveShellFolder.h"

#include "Logging\BigDriveShellFolderTraceLogger.h"

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

    m_traceLogger.LogEnter(__FUNCTION__, m_driveGuid);

    if (!pClassID)
    {
        hr = E_POINTER;
        s_eventLogger.WriteErrorFormmated(L"GetClassID: Invalid Pointer. HRESULT: 0x%08X", hr);
        goto End;
    }

    *pClassID = m_driveGuid;

End:

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}
