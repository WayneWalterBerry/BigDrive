// <copyright file="COMAdminCatalog.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <oaidl.h>

// Local
#include "ApplicationCollection.h"
#include "Dispatch.h"
#include "Interfaces/ICOMAdminCatalog2.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\COMAdminCatalogTests.h"

class COMAdminCatalog : public Dispatch
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;


    COMAdminCatalog(LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
    }

public:

    static HRESULT Create(COMAdminCatalog **ppCOMAdminCatalog);
    
    HRESULT GetApplicationsCollection(ApplicationCollection** ppApplicationCollection);

    HRESULT GetComponentCollection(Application *pApplication, ComponentCollection **ppComponentCollection);

    HRESULT GetCollectionByQuery(LPWSTR collectionName, BSTR appKey, IDispatch** pIDispatch);

    HRESULT GetICOMAdminCatalog2(ICOMAdminCatalog2** ppICOMAdminCatalog2)
    {
        return m_pIDispatch->QueryInterface(IID_ICOMAdminCatalog2, (void**)&ppICOMAdminCatalog2);
    }

    friend class BigDriveClientTest::COMAdminCatalogTests;
};