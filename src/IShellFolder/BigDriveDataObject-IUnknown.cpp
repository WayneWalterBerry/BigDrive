// <copyright file="BigDriveDataObject-IUnknown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveDataObject.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

/// <summary>
/// QueryInterface implementation for the BigDriveDataObject.
/// </summary>
/// <param name="riid">The interface ID requested.</param>
/// <param name="ppv">Address of pointer variable that receives the interface pointer.</param>
/// <returns>S_OK if successful, E_NOINTERFACE if the interface is not supported.</returns>
HRESULT __stdcall BigDriveDataObject::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    m_traceLogger.LogEnter(__FUNCTION__, riid, m_apidl, m_cidl);

    if (!ppv)
    {
        return E_POINTER;
    }

    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDataObject))
    {
        *ppv = static_cast<IDataObject*>(this);
        AddRef();
        hr = S_OK;
    }

    m_traceLogger.LogExit(__FUNCTION__, hr);

    return hr;
}

/// <summary>
/// AddRef implementation for the BigDriveDataObject.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG __stdcall BigDriveDataObject::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

/// <summary>
/// Release implementation for the BigDriveDataObject.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG __stdcall BigDriveDataObject::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}