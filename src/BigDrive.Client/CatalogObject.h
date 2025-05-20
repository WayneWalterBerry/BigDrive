// <copyright file="CatalogObject.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"
#include "Interfaces/ICatalogObject.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class CatalogObject : public Dispatch
{

public:

    CatalogObject(LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
    }

    ~CatalogObject()
    {
    }

    // ==================== ICatalogObject ====================

    HRESULT GetName(BSTR& bstrName);

    HRESULT GetDescription(BSTR& bstrDescription);
    HRESULT GetKey(BSTR& bstrKey);

    friend class BigDriveClientTest::ApplicationTests;

protected:

    HRESULT GetICatalogObject(ICatalogObject** ppICatalogObject)
    {
        return m_pIDispatch->QueryInterface(IID_ICatalogObject, (void**)ppICatalogObject);
    }
};