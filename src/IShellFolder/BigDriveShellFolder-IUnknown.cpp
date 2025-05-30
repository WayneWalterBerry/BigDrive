// <copyright file="BigDriveShellFolder-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"
#include "BigDriveShellFolderTraceLogger.h"

/// <summary>
/// Queries the object for a pointer to one of its supported interfaces.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::QueryInterface(REFIID riid, void** ppvObject)
{
    HRESULT hr = S_OK;

    BigDriveShellFolderTraceLogger::LogEnter(__FUNCTION__, riid);

    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IShellFolder*>(this);
        AddRef();
        hr = S_OK;
        goto End;
    }
    if (riid == IID_IShellFolder)
    {
        *ppvObject = static_cast<IShellFolder*>(this);
        AddRef();
        hr = S_OK;
        goto End;
    }
    else if (IsEqualIID(riid, IID_IShellFolder2))
    {
        *ppvObject = static_cast<IShellFolder2*>(this);
        AddRef();
        hr = S_OK;
        goto End;
    }
    else if (IsEqualIID(riid, IID_IPersistFolder) || IsEqualIID(riid, IID_IPersistFolder2) || IsEqualIID(riid, IID_IPersist)) 
    {
        *ppvObject = static_cast<IPersistFolder2*>(this);
        AddRef();
        hr = S_OK;
        goto End;
    }
    else if (IsEqualIID(riid, IID_IObjectWithBackReferences))
    {
        *ppvObject = static_cast<IObjectWithBackReferences*>(this);
        AddRef();
        hr = S_OK;
        goto End;
    }
    else if (IsEqualIID(riid, IID_IProvideClassInfo))
    {
        *ppvObject = static_cast<IProvideClassInfo*>(this);
        AddRef();
        hr = S_OK;
        goto End;
	}
    else if (IsEqualIID(riid, IID_IExtractIconW) || IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIcon))
    {
        *ppvObject = static_cast<IExtractIconW*>(this);
        AddRef();
        hr = S_OK;
        goto End;
    }

    *ppvObject = nullptr;
    hr = E_NOINTERFACE;

End:

    BigDriveShellFolderTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}

/// <summary>
/// Increments the reference count for the object.
/// </summary>
ULONG __stdcall BigDriveShellFolder::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

/// <summary>
/// Decrements the reference count for the object. Deletes the object if the reference count reaches zero.
/// </summary>
ULONG __stdcall BigDriveShellFolder::Release()
{
    LONG ref = InterlockedDecrement(&m_refCount);
    if (ref == 0)
    {
        delete this;
    }
    return ref;
}