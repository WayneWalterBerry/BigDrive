// <copyright file="ComponentCollection.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "CatalogCollection.h"
#include "Component.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class ComponentCollection : public CatalogCollection
{

public:

    ComponentCollection(LPDISPATCH pDispatch)
        : CatalogCollection(pDispatch)
    {
    }

    ~ComponentCollection()
    {
    }

    HRESULT GetName(BSTR& bstrString);

    HRESULT GetComponents(Component*** pppComponents, long& lSize);

    friend class BigDriveClientTest::ApplicationTests;
};
