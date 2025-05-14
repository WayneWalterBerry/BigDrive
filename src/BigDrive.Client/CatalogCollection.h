// <copyright file="CatalogCollection.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"

// Shared
#include "..\Shared\EventLogger.h"

// Local

#include "Dispatch.h"

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
};
