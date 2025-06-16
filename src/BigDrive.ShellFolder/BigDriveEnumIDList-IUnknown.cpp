// <copyright file="IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
//   Implements the BigDriveEnumIDList class, a custom IEnumIDList for enumerating PIDLs
//   in the BigDrive shell extension.
// </summary>

#include "pch.h"

#include "BigDriveEnumIDList.h"
#include <shlobj.h>

/// <summary>
/// Queries for a supported interface (IUnknown or IEnumIDList).
/// </summary>
HRESULT __stdcall BigDriveEnumIDList::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown || riid == IID_IEnumIDList)
    {
        *ppv = static_cast<IEnumIDList*>(this);
        AddRef();
        return S_OK;
    }

    *ppv = nullptr;
    return E_NOINTERFACE;
}

/// <summary>
/// Increments the reference count.
/// </summary>
ULONG __stdcall BigDriveEnumIDList::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

/// <summary>
/// Decrements the reference count and deletes the object if it reaches zero.
/// </summary>
ULONG __stdcall BigDriveEnumIDList::Release()
{
    ULONG res = InterlockedDecrement(&m_refCount);
    if (res == 0) delete this;
    return res;
}