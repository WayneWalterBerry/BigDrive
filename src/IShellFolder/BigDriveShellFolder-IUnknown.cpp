// <copyright file="BigDriveShellFolder-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"

/// <summary>
/// Queries the object for a pointer to one of its supported interfaces.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::QueryInterface(REFIID riid, void** ppvObject)
{
    if (riid == IID_IUnknown || riid == IID_IShellFolder)
    {
        *ppvObject = static_cast<IShellFolder*>(this);
        AddRef();
        return S_OK;
    }

    s_eventLogger.WriteErrorFormmated(L"BigDriveShellFolder::QueryInterface", L"Unknown interface requested: %s", riid.Data1);

    *ppvObject = nullptr;
    return E_NOINTERFACE;
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