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
#include "BigDriveClientEventLogger.h"

// Forward Declarations Of Test Classes
#include "..\..\test\unit\BigDrive.Client.Test\COMAdminCatalogTests.h"

class COMAdminCatalog : public Dispatch
{
private:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static BigDriveClientEventLogger s_eventLogger;


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

    /// <summary>
    /// Starts the specified COM+ application using the COMAdminCatalog.
    /// Retrieves the application's name, obtains the ICOMAdminCatalog2 interface,
    /// and calls StartApplication to launch the application. Logs errors if any step fails.
    /// </summary>
    /// <param name="pApplication">Pointer to the Application object to start.</param>
    /// <returns>HRESULT indicating success or failure of the operation.</returns>
    HRESULT Start(Application* pApplication);

    /// <summary>
    /// Searches for a COM+ application by its name in the catalog.
    /// Retrieves the applications collection and calls QueryApplicationByName to find an application
    /// whose name matches the specified value. If found, clones the application into ppApplication and returns S_OK.
    /// Returns HRESULT_FROM_WIN32(ERROR_NOT_FOUND) if no match is found, or an error HRESULT on failure.
    /// Logs errors using the event logger.
    /// </summary>
    HRESULT QueryApplicationByName(LPWSTR bstrName, Application** ppApplication);

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