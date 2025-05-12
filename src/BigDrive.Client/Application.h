// <copyright file="Application.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "CatalogCollection.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class Application : CatalogCollection
{

public:

    Application(LPDISPATCH pDispatch)
        : CatalogCollection(pDispatch)
    {
    }

    ~Application()
    {
    }

    HRESULT GetName(BSTR& bstrString);

    HRESULT GetComponentsCLSIDs(CLSID** ppCLSIDs, long& lSize);

    friend class BigDriveClientTest::ApplicationTests;
};
