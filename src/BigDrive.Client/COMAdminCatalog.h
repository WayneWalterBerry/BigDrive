// <copyright file="COMAdminCatalog.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comadmin.h>
#include <oaidl.h>

#include "ApplicationCollection.h"
#include "Dispatch.h"

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
};