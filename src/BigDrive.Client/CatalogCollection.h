// <copyright file="CatalogCollection.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"

// Shared
#include "..\Shared\EventLogger.h"

// Local

#include "COMAdmin.h"

class CatalogCollection : public COMAdmin
{

public:

    CatalogCollection(LPDISPATCH pDispatch)
        : COMAdmin(pDispatch)
    {
    }

    ~CatalogCollection()
    {
    }

    HRESULT Populate();

    HRESULT GetCount(LONG& lCount);

    HRESULT GetName(BSTR& bstrName);
};
