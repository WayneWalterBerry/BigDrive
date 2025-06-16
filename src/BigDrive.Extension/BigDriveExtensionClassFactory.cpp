// <copyright file="BigDriveExtensionClassFactory.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveExtensionClassFactory.h"

/// <inheritdoc/>
BigDriveExtensionClassFactory::BigDriveExtensionClassFactory()
    : m_cRef(1)
{

}

/// <inheritdoc/>
BigDriveExtensionClassFactory::~BigDriveExtensionClassFactory()
{

}

/// <inheritdoc/>
STDMETHODIMP BigDriveExtensionClassFactory::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv == nullptr)
    {
        hr = E_POINTER;
        goto End;
    }

    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_IClassFactory)
    {
        *ppv = static_cast<IClassFactory*>(this);
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

End:

    return hr;

}

/// <inheritdoc/>
STDMETHODIMP_(ULONG) BigDriveExtensionClassFactory::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

/// <inheritdoc/>
STDMETHODIMP_(ULONG) BigDriveExtensionClassFactory::Release()
{
    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

/// <inheritdoc/>
STDMETHODIMP BigDriveExtensionClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
    HRESULT hr = S_OK;
    BigDriveExtension* pExt = nullptr;

    if (ppvObject == nullptr)
    {
        hr = E_POINTER;
        goto End;
    }

    *ppvObject = nullptr;

    if (pUnkOuter != nullptr)
    {
        hr = CLASS_E_NOAGGREGATION;
        goto End;
    }

    pExt = new BigDriveExtension();
    if (!pExt)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    hr = pExt->QueryInterface(riid, ppvObject);

End:

    if (pExt)
    {
        pExt->Release();
        pExt = nullptr;
    }

    return hr;

}

/// <inheritdoc/>
STDMETHODIMP BigDriveExtensionClassFactory::LockServer(BOOL fLock)
{
    UNREFERENCED_PARAMETER(fLock);
    return S_OK;
}