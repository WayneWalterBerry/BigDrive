// <copyright file="BigDriveExtension-IUnkown.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveExtension.h"

STDMETHODIMP BigDriveExtension::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv == nullptr)
    {
        hr = E_POINTER;
        goto End;
    }

    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_IContextMenu)
    {
        *ppv = static_cast<IContextMenu*>(this);
    }
    else if (riid == IID_IShellExtInit)
    {
        *ppv = static_cast<IShellExtInit*>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        goto End;
    }

    AddRef();

End:

    return hr;
}

STDMETHODIMP_(ULONG) BigDriveExtension::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) BigDriveExtension::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}