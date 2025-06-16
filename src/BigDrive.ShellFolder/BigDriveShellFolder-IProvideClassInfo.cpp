// <copyright file="BigDriveShellFolder-IProvideClassInfo.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>
// <summary>
//   Minimal implementation of the IProvideClassInfo interface for the BigDriveShellFolder class.
//   IProvideClassInfo allows clients to retrieve type information (ITypeInfo) for the COM class.
//   For most shell folder extensions, a stub implementation that returns E_NOTIMPL is sufficient,
//   as type information is rarely required or used by the shell or consumers.
// </summary>

#include "pch.h"

#include "BigDriveShellFolder.h"

#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <ocidl.h> // For IProvideClassInfo
#include <objbase.h>
#include <windows.h>

/// <summary>
/// Retrieves the ITypeInfo interface for the class.
/// The Windows Shell or other clients may call this method to obtain type information
/// for automation or introspection purposes. In the context of a shell folder extension,
/// this is rarely needed, so a minimal implementation simply returns E_NOTIMPL.
/// </summary>
/// <param name="ppTI">Address of a pointer that receives the ITypeInfo interface pointer.</param>
/// <returns>E_NOTIMPL to indicate that type information is not provided.</returns>
/// <remarks>
/// Returning E_NOTIMPL is standard for shell extensions that do not expose automation
/// or type information. If you need to provide type info for scripting or automation,
/// implement this method to return a valid ITypeInfo pointer.
/// </remarks>
HRESULT __stdcall BigDriveShellFolder::GetClassInfo(ITypeInfo** ppTI)
{
    m_traceLogger.LogEnter(__FUNCTION__);

    // No type information provided in this minimal implementation.
    HRESULT hr = E_NOTIMPL;
    if (ppTI)
    {
        *ppTI = nullptr;
    }

    m_traceLogger.LogExit(__FUNCTION__, hr);
    return hr;
}