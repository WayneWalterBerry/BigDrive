// <copyright file="ApplicationManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <comdef.h>
#include <iostream>

// Header
#include "ApplicationManager.h"

// Local
#include "Dispatch.h"

// Initialize the static EventLogger instance
EventLogger ApplicationManager::s_eventLogger(L"BigDrive.Client");

HRESULT ApplicationManager::StartApplication(CLSID clsidProvider)
{
    HRESULT hrReturn = S_OK;
    CLSID clsidCOMAdminCatalog;
    IDispatch* pIDispatchCatalog = nullptr;
    DISPID dispid;
    LPOLESTR progID = nullptr;
    const OLECHAR* methodName = L"StartApplication";

    // Invoke StartApplication
    DISPPARAMS params = {};
    VARIANTARG varg = {};

    varg.vt = VT_BSTR;
    params.rgvarg = &varg;
    params.cArgs = 1;

    hrReturn = ::ProgIDFromCLSID(clsidProvider, &progID);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to retrieve ProgID from CLSID. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    varg.bstrVal = progID;

    // Create COMAdminCatalog instance
    hrReturn = ::CLSIDFromProgID(L"COMAdmin.COMAdminCatalog", &clsidCOMAdminCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to retrieve CLSID for COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = ::CoCreateInstance(clsidCOMAdminCatalog, nullptr, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pIDispatchCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to create COMAdminCatalog instance. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Prepare arguments for StartApplication
    hrReturn = pIDispatchCatalog->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&methodName), 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to get DISPID for StartApplication. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = pIDispatchCatalog->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to invoke StartApplication. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

End:

    // Cleanup
    if (progID != nullptr)
    {
        ::CoTaskMemFree(progID);
        progID = nullptr;
    }

    if (pIDispatchCatalog != nullptr)
    {
        pIDispatchCatalog->Release();
        pIDispatchCatalog = nullptr;
    }

    return hrReturn;
}

/// <summary>
/// Retrieves the COMAdminCatalog object, which provides access to COM+ administration.
/// </summary>
/// <param name="ppIDispatchCatalog">Pointer to an IDispatch pointer that will receive the COMAdminCatalog object.</param>
/// <returns>HRESULT indicating success or failure of the operation.</returns>
/// </summary>
HRESULT ApplicationManager::GetCOMAdminCatalog(IDispatch** ppIDispatchCatalog)
{
    HRESULT hrReturn = S_OK;

    if (ppIDispatchCatalog == nullptr)
    {
        s_eventLogger.WriteError(L"GetCOMAdminCatalog: Invalid pointer passed for ppIDispatchCatalog.");
        return E_POINTER;
    }

    CLSID clsidCOMAdminCatalog;

    // Retrieve the CLSID for COMAdminCatalog
    hrReturn = ::CLSIDFromProgID(L"COMAdmin.COMAdminCatalog", &clsidCOMAdminCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCOMAdminCatalog: Failed to retrieve CLSID for COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Create an instance of the COMAdminCatalog
    hrReturn = ::CoCreateInstance(clsidCOMAdminCatalog, nullptr, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)ppIDispatchCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCOMAdminCatalog: Failed to create an instance of COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

End:

    return hrReturn;
}

/// <summary>
/// Retrieves the Applications collection from the COM+ catalog.
/// </summary>
/// <param name="ppIDispatchApplicationsCollection">Pointer to an IDispatch pointer that will receive the Applications collection.</param>
/// <returns>HRESULT indicating success or failure of the operation.</returns>
HRESULT ApplicationManager::GetApplicationsCollection(IDispatch** ppIDispatchApplicationsCollection)
{
    HRESULT hrReturn = S_OK;

    if (!ppIDispatchApplicationsCollection)
    {
        s_eventLogger.WriteError(L"GetApplicationsCollection: Invalid pointer passed for ppIDispatchApplicationsCollection.");
        return E_POINTER;
    }

    IDispatch* pIDispatchCatalog = nullptr;
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

    hrReturn = GetCOMAdminCatalog(&pIDispatchCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to get COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Get the DISPID for GetCollection method
    hrReturn = pIDispatchCatalog->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&methodName), 1, LOCALE_USER_DEFAULT, &dispidGetCollection);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to get DISPID for GetCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = pIDispatchCatalog->Invoke(dispidGetCollection, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &vtCollections, nullptr, nullptr);
    if (FAILED(hrReturn) || (vtCollections.vt != VT_DISPATCH))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to invoke GetCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    *ppIDispatchApplicationsCollection = vtCollections.pdispVal;

End:

    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Operation failed. HRESULT: 0x%08X", hrReturn);
    }

    if (pIDispatchCatalog)
    {
        pIDispatchCatalog->Release();
        pIDispatchCatalog = nullptr;
    }

    if (varg.bstrVal)
    {
        ::SysFreeString(varg.bstrVal);
    }

    return hrReturn;
}



