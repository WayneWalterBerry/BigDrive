// <copyright file="COMAdminCatalog.h" company="Wayne Walter Berry">
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

    /// <summary>
    /// Creates an instance of the COMAdminCatalog class and initializes it with a COMAdminCatalog COM object.
    /// </summary>
    /// <param name="ppCOMAdminCatalog">Address of a pointer that receives the created COMAdminCatalog instance.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    static HRESULT Create(COMAdminCatalog **ppCOMAdminCatalog);
    
    HRESULT GetApplicationsCollection(ApplicationCollection** ppApplicationCollection);

    /// <summary>
    /// Retrieves the collection of COM+ components for a specified application.
    /// </summary>
    /// <param name="pApplication">Pointer to the Application object whose components are to be retrieved.</param>
    /// <param name="ppComponentCollection">Address of a pointer that receives the resulting ComponentCollection object.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    HRESULT GetComponentCollection(Application *pApplication, ComponentCollection **ppComponentCollection);

    HRESULT GetCollectionByQuery(LPWSTR collectionName, BSTR appKey, IDispatch** pIDispatch);

    friend class BigDriveClientTest::COMAdminCatalogTests;

private:

    HRESULT GetICOMAdminCatalog2(ICOMAdminCatalog2** ppICOMAdminCatalog2)
    {
        if (!ppICOMAdminCatalog2)
        {
            return E_POINTER;
        }

        return m_pIDispatch->QueryInterface(IID_ICOMAdminCatalog2, (void**)ppICOMAdminCatalog2);
    }
};