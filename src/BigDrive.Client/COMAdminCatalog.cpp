// <copyright file="COMAdminCatalog.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comadmin.h>
#include <oaidl.h>
#include <comdef.h>

// Header
#include "COMAdminCatalog.h"

// Local
#include "ApplicationCollection.h"
#include "ComponentCollection.h"

// Initialize the static EventLogger instance
EventLogger COMAdminCatalog::s_eventLogger(L"BigDrive.Client");

HRESULT COMAdminCatalog::Create(COMAdminCatalog **ppCOMAdminCatalog)
{
    HRESULT hrReturn = S_OK;

    LPDISPATCH pIDispatch = nullptr;

    CLSID clsidCOMAdminCatalog;

    // Retrieve the CLSID for COMAdminCatalog
    hrReturn = ::CLSIDFromProgID(L"COMAdmin.COMAdminCatalog", &clsidCOMAdminCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCOMAdminCatalog: Failed to retrieve CLSID for COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Create an instance of the COMAdminCatalog
    hrReturn = ::CoCreateInstance(clsidCOMAdminCatalog, nullptr, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pIDispatch);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCOMAdminCatalog: Failed to create an instance of COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    *ppCOMAdminCatalog = new COMAdminCatalog(pIDispatch);

End:

    return hrReturn;
}

HRESULT COMAdminCatalog::GetApplicationsCollection(ApplicationCollection** ppApplicationCollection)
{
    HRESULT hrReturn = S_OK;

    if (!ppApplicationCollection)
    {
        s_eventLogger.WriteError(L"GetApplicationsCollection: Invalid pointer passed for ppIDispatchApplicationsCollection.");
        return E_POINTER;
    }

    DISPID dispidGetCollection;
    const OLECHAR* methodName = L"GetCollection";

    VARIANT vtCollections;
    ::VariantInit(&vtCollections);
    DISPPARAMS params = { nullptr, nullptr, 0, 0 };

    VARIANTARG varg;
    varg.vt = VT_BSTR;
    varg.bstrVal = ::SysAllocString(L"Applications"); // Request Collection list
    params.rgvarg = &varg;
    params.cArgs = 1;

    // Get the DISPID for GetCollection method
    hrReturn = GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&methodName), 1, LOCALE_USER_DEFAULT, &dispidGetCollection);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to get DISPID for GetCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = Invoke(dispidGetCollection, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &vtCollections, nullptr, nullptr);
    if (FAILED(hrReturn) || (vtCollections.vt != VT_DISPATCH))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to invoke GetCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    *ppApplicationCollection = new ApplicationCollection(vtCollections.pdispVal);

End:

    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Operation failed. HRESULT: 0x%08X", hrReturn);
    }


    if (varg.bstrVal)
    {
        ::SysFreeString(varg.bstrVal);
    }

    return hrReturn;
}



