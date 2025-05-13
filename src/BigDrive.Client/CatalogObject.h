// <copyright file="CatalogObject.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"

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

    // ==================== ICatalogObject Methods ====================

    HRESULT GetName(BSTR& bstrName);
    HRESULT GetId(BSTR& bstrName);
    HRESULT GetDescription(BSTR& bstrDescription);
};