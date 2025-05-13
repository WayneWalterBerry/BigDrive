// <copyright file="Component.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "CatalogCollection.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class Component : CatalogCollection
{

public:

    Component(COMAdminCatalog* pCOMAdminCatalog, LPDISPATCH pDispatch)
        : CatalogCollection(pCOMAdminCatalog, pDispatch)
    {
    }

    ~Component()
    {
    }

    friend class BigDriveClientTest::ApplicationTests;
}; 