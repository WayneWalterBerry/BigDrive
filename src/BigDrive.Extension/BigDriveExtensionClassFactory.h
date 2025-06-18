// <copyright file="BigDriveExtensionClassFactory.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "pch.h"
#include <windows.h>
#include <unknwn.h>
#include "BigDriveExtension.h"

/// <summary>
/// Class factory for BigDriveExtension.
/// Implements IClassFactory to create instances of BigDriveExtension.
/// </summary>
class BigDriveExtensionClassFactory : public IClassFactory
{
public:

    /// <summary>
    /// Standard constructor.
    /// </summary>
    BigDriveExtensionClassFactory();

    /// <summary>
    /// Standard destructor.
    /// </summary>
    virtual ~BigDriveExtensionClassFactory();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    STDMETHODIMP LockServer(BOOL fLock) override;

private:

    LONG m_cRef;

};