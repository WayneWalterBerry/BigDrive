// <copyright file="BigDriveTransferSource-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveTransferSource.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

/// <summary>
/// QueryInterface implementation for the BigDriveTransferSource.
/// </summary>
/// <param name="riid">The interface ID requested.</param>
/// <param name="ppv">Address of pointer variable that receives the interface pointer.</param>
/// <returns>S_OK if successful, E_NOINTERFACE if the interface is not supported.</returns>
HRESULT __stdcall BigDriveTransferSource::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITransferSource))
    {
        *ppv = static_cast<ITransferSource*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

/// <summary>
/// AddRef implementation for the BigDriveTransferSource.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG __stdcall BigDriveTransferSource::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

/// <summary>
/// Release implementation for the BigDriveTransferSource.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG __stdcall BigDriveTransferSource::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}