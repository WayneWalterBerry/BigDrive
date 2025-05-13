// <copyright file="Application.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "CatalogObject.h"
#include "ComponentCollection.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\ApplicationTests.h"

class Application : public CatalogObject
{

public:

    Application(COMAdminCatalog* pCOMAdminCatalog, LPDISPATCH pDispatch)
        : CatalogObject(pCOMAdminCatalog, pDispatch)
    {
    }

    ~Application()
    {
    }


    HRESULT GetComponentCollection(ComponentCollection** ppComponentCollection);

    friend class BigDriveClientTest::ApplicationTests;
};
