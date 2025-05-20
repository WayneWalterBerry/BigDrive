// <copyright file="CatalogCollection.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"
#include "Interfaces/ICatalogCollection.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationCollectionTests.h"

class CatalogCollection : public Dispatch
{

public:

    CatalogCollection(LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
    }

    ~CatalogCollection()
    {
    }

    HRESULT Populate();

    HRESULT GetCount(LONG& lCount);

    HRESULT GetName(BSTR& bstrName);

    friend class BigDriveClientTest::ApplicationCollectionTests;

protected:

    HRESULT GetICatalogCollection(ICatalogCollection** ppICatalogCollection)
    {
        return m_pIDispatch->QueryInterface(IID_ICatalogCollection, (void**)ppICatalogCollection);
    }
};
