// <copyright file="BigDriveDropTarget-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveDropTarget.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

/// <summary>
/// QueryInterface implementation for the BigDriveDropTarget.
/// </summary>
/// <param name="riid">The interface ID requested.</param>
/// <param name="ppv">Address of pointer variable that receives the interface pointer.</param>
/// <returns>S_OK if successful, E_NOINTERFACE if the interface is not supported.</returns>
HRESULT __stdcall BigDriveDropTarget::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv)
        return E_POINTER;

    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
        *ppv = static_cast<IDropTarget*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

/// <summary>
/// AddRef implementation for the BigDriveDropTarget.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG __stdcall BigDriveDropTarget::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

/// <summary>
/// Release implementation for the BigDriveDropTarget.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG __stdcall BigDriveDropTarget::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}