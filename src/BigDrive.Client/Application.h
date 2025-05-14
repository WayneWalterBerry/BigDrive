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

    Application(LPDISPATCH pDispatch)
        : CatalogObject( pDispatch)
    {
    }

    ~Application()
    {
    }

    HRESULT Clone(Application** ppApplication) const
    {
        if (ppApplication == nullptr)
        {
            return E_POINTER;
        }

        *ppApplication = new Application(m_pIDispatch);

        return S_OK;
    }

    friend class BigDriveClientTest::ApplicationTests;
};
