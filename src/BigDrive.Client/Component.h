// <copyright file="Component.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "CatalogObject.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class Component : public CatalogObject
{

public:

    Component(LPDISPATCH pDispatch)
        : CatalogObject(pDispatch)
    {
    }

    ~Component()
    {
    }

    /// <summary>
    /// Retrieves the CLSID value of the component and returns it as a CLSID value.
    /// </summary>
    /// <param name="clsid">Reference to a CLSID that receives the component's CLSID.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetCLSID(CLSID& clsid)
    {
        return GetValue(L"CLSID", clsid);
    }

    /// <summary>
    /// Creates a new Component object that shares the same underlying IDispatch pointer as this instance.
    /// </summary>
    /// <param name="ppComponent">Address of a pointer that receives the newly created Component object.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT Clone(Component** ppComponent) const
    {
        if (ppComponent == nullptr)
        {
            return E_POINTER;
        }

        *ppComponent = new Component(m_pIDispatch);

        return S_OK;
    }

    friend class BigDriveClientTest::ApplicationTests;
}; 