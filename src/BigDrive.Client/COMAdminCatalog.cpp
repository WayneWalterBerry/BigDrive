// <copyright file="COMAdminCatalog.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <oaidl.h>
#include <comdef.h>

// Header
#include "COMAdminCatalog.h"

// Local
#include "ApplicationCollection.h"
#include "ComponentCollection.h"
#include "VariantUtil.h"
#include "COMUtility.h"

// Initialize the static EventLogger instance
EventLogger COMAdminCatalog::s_eventLogger(L"BigDrive.Client");

HRESULT COMAdminCatalog::Create(COMAdminCatalog** ppCOMAdminCatalog)
{
    HRESULT hr = S_OK;

    LPDISPATCH pIDispatch = nullptr;

    CLSID clsidCOMAdminCatalog;

    // Retrieve the CLSID for COMAdminCatalog
    hr = ::CLSIDFromProgID(L"COMAdmin.COMAdminCatalog", &clsidCOMAdminCatalog);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCOMAdminCatalog: Failed to retrieve CLSID for COMAdminCatalog. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Create an instance of the COMAdminCatalog
    hr = ::CoCreateInstance(clsidCOMAdminCatalog, nullptr, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pIDispatch);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCOMAdminCatalog: Failed to create an instance of COMAdminCatalog. HRESULT: 0x%08X", hr);
        goto End;
    }

    *ppCOMAdminCatalog = new COMAdminCatalog(pIDispatch);

End:

    return hr;
}

HRESULT COMAdminCatalog::GetApplicationsCollection(ApplicationCollection** ppApplicationCollection)
{
    HRESULT hr = S_OK;
    ICOMAdminCatalog2* pICOMAdminCatalog2 = nullptr;
    IDispatch* pIDispatchCollection = nullptr;
    BSTR bstrCollectionName = nullptr;

    if (!ppApplicationCollection)
    {
        s_eventLogger.WriteError(L"GetApplicationsCollection: Invalid pointer passed for ppIDispatchApplicationsCollection.");
        return E_POINTER;
    }

    bstrCollectionName = ::SysAllocString(L"Applications");

    hr = GetICOMAdminCatalog2(&pICOMAdminCatalog2);
    if (FAILED(hr) || (pICOMAdminCatalog2 == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to get ICOMAdminCatalog2 HRESULT: 0x%08X", hr);
        goto End;
    }

    // Call GetCollection method to retrieve the Applications collection
    hr = pICOMAdminCatalog2->GetCollection(bstrCollectionName, (IDispatch**)&pIDispatchCollection);
    if (FAILED(hr) || (pIDispatchCollection == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed call to ICOMAdminCatalog2::GetCollection. HRESULT: 0x%08X", hr);
        goto End;
    }

    *ppApplicationCollection = new ApplicationCollection(pIDispatchCollection);

End:

    if (pIDispatchCollection)
    {
        pIDispatchCollection->Release();
        pIDispatchCollection = nullptr;
    }

    if (bstrCollectionName)
    {
        ::SysFreeString(bstrCollectionName);
        bstrCollectionName = nullptr;
    }

    if (pICOMAdminCatalog2 == NULL)
    {
        pICOMAdminCatalog2->Release();
        pICOMAdminCatalog2 = nullptr;
    }

    return hr;
}

/// </inheritdoc>
HRESULT COMAdminCatalog::GetComponentCollection(Application* pApplication, ComponentCollection** ppComponentCollection)
{
    HRESULT hr = S_OK;
    ICOMAdminCatalog2* pICOMAdminCatalog2 = nullptr;
    IDispatch* pIDispatchCollection = nullptr;
    BSTR bstrCollectionName = nullptr;
    BSTR bstrAppId = nullptr;
    SAFEARRAY* psaQuery = nullptr;

    if (!ppComponentCollection)
    {
        s_eventLogger.WriteError(L"GetApplicationsCollection: Invalid pointer passed for ppIDispatchApplicationsCollection.");
        return E_POINTER;
    }

    bstrCollectionName = ::SysAllocString(L"Components");

    hr = GetICOMAdminCatalog2(&pICOMAdminCatalog2);
    if (FAILED(hr) || (pICOMAdminCatalog2 == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to get ICOMAdminCatalog2 HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pApplication->GetId(bstrAppId);
    if (FAILED(hr) || (bstrAppId == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to get Application ID. HRESULT: 0x%08X", hr);
        return hr;
    }

    hr = CreateSafeArrayFromBSTR(bstrAppId, &psaQuery);
    if (FAILED(hr) || (psaQuery == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed to create SafeArray from BSTR. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Call GetCollection method to retrieve the Applications collection
    hr = pICOMAdminCatalog2->GetCollectionByQuery(bstrCollectionName, &psaQuery, (IDispatch**)&pIDispatchCollection);
    if (FAILED(hr) || (pIDispatchCollection == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection: Failed call to ICOMAdminCatalog2::GetCollection. HRESULT: 0x%08X", hr);
        goto End;
    }

    *ppComponentCollection = new ComponentCollection(pIDispatchCollection);

End:


    if (psaQuery)
    {
        ::SafeArrayDestroy(psaQuery);
        psaQuery = nullptr;
    }

    if (bstrAppId)
    {
        ::SysFreeString(bstrAppId);
        bstrAppId = nullptr;
    }

    if (pIDispatchCollection)
    {
        pIDispatchCollection->Release();
        pIDispatchCollection = nullptr;
    }

    if (bstrCollectionName)
    {
        ::SysFreeString(bstrCollectionName);
        bstrCollectionName = nullptr;
    }

    if (pICOMAdminCatalog2 == NULL)
    {
        pICOMAdminCatalog2->Release();
        pICOMAdminCatalog2 = nullptr;
    }

    return hr;
}

HRESULT COMAdminCatalog::GetCollectionByQuery(LPWSTR szCollectionName, BSTR appKey, IDispatch** pIDispatch)
{
    if (!pIDispatch || !szCollectionName || !appKey)
    {
        return E_POINTER;
    }

    // Prepare query array
    SAFEARRAY* pQueryArray = SafeArrayCreateVector(VT_VARIANT, 0, 1);

    VARIANT varKey;
    ::VariantInit(&varKey, appKey);

    VARIANT varCollectionName;
    ::VariantInit(&varCollectionName, szCollectionName);

    LONG index = 0;
    ::SafeArrayPutElement(pQueryArray, &index, &varKey);

    // Call GetCollectionByQuery
    DISPID dispidGetCollectionByQuery;
    LPOLESTR szGetCollectionByQuery = ::SysAllocString(L"GetCollectionByQuery");
    HRESULT hr = GetIDsOfNames(IID_NULL, &szGetCollectionByQuery, 1, LOCALE_USER_DEFAULT, &dispidGetCollectionByQuery);

    if (SUCCEEDED(hr))
    {
        VARIANT varQueryArray;
        ::VariantInit(&varQueryArray);
        varQueryArray.vt = VT_ARRAY | VT_VARIANT;
        varQueryArray.parray = pQueryArray;

        DISPPARAMS params = { new VARIANT[2]{ varCollectionName, varQueryArray }, nullptr, 2, 0 };
        VARIANT varResult;
        VariantInit(&varResult);

        hr = Invoke(dispidGetCollectionByQuery, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, nullptr, nullptr);
        SafeArrayDestroy(pQueryArray);

        if (SUCCEEDED(hr) && varResult.vt == VT_DISPATCH)
        {
            *pIDispatch = varResult.pdispVal;
            (*pIDispatch)->AddRef();
        }

        VariantClear(&varResult);
    }

    ::SysFreeString(szGetCollectionByQuery);

    ::VariantClear(&varCollectionName);
    ::VariantClear(&varKey);

    return S_OK;
}

