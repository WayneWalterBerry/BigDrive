// <copyright file="BigDriveShellIcon-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellIcon.h"

// Local
#include "LaunchDebugger.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

/// <summary>
/// Queries the object for a pointer to one of its supported interfaces.
/// </summary>
HRESULT __stdcall BigDriveShellIcon::QueryInterface(REFIID riid, void** ppvObject)
{
    HRESULT hr = S_OK;

    m_traceLogger.LogEnter(__FUNCTION__, riid);

    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IExtractIconW*>(this);
        AddRef();
        goto End;
    }
    else if (IsEqualIID(riid, IID_IExtractIconW))
    {
        *ppvObject = static_cast<IExtractIconW*>(this);
        AddRef();
        goto End;
    }
    else if (IsEqualIID(riid, IID_IExtractIconA))
    {
        *ppvObject = static_cast<IExtractIconA*>(this);
        AddRef();
        goto End;
    }

    *ppvObject = nullptr;
    hr = E_NOINTERFACE;

End:

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}

/// <summary>
/// Increments the reference count for the object.
/// </summary>
ULONG __stdcall BigDriveShellIcon::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

/// <summary>
/// Decrements the reference count for the object. Deletes the object if the reference count reaches zero.
/// </summary>
ULONG __stdcall BigDriveShellIcon::Release()
{
    LONG ref = InterlockedDecrement(&m_refCount);
    if (ref == 0)
    {
        delete this;
    }
    return ref;
}