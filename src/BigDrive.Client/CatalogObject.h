// <copyright file="CatalogObject.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"
#include "Interfaces/ICatalogCollection.h"

// Shared
#include "..\Shared\EventLogger.h"

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

    HRESULT GetICatalogCollection(ICatalogCollection** ppICatalogCollection)
    {
        return m_pIDispatch->QueryInterface(IID_ICatalogCollection, (void**)ppICatalogCollection);
    }

    // ==================== ICatalogObject ====================

    HRESULT GetName(BSTR& bstrName);
    HRESULT GetId(BSTR& bstrName);
    HRESULT GetDescription(BSTR& bstrDescription);
    HRESULT GetKey(BSTR& bstrKey);
};