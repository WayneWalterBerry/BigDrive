// <copyright file="CatalogCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "CatalogCollection.h"

HRESULT CatalogCollection::Populate()
{
    HRESULT hr = S_OK;

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    DISPID dispidPopulate;
    const OLECHAR* populateMethod = L"Populate";

    // Get the DISPID for the "Populate" method
    LPOLESTR mutablePopulateMethod = const_cast<LPOLESTR>(populateMethod);

    hr = GetIDsOfNames(IID_NULL, &mutablePopulateMethod, 1, LOCALE_USER_DEFAULT, &dispidPopulate);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Failed to get DISPID for 'Populate'. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Invoke the "Populate" method
    hr = m_pIDispatch->Invoke(dispidPopulate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Failed to invoke 'Populate'. HRESULT: 0x%08X", hr);
        goto End;
    }

End:

    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Operation failed. HRESULT: 0x%08X", hr);
    }

    return hr;
}

HRESULT CatalogCollection::GetCount(LONG& lCount)
{
    HRESULT hr = S_OK;

    hr = Populate();
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate collection. HRESULT: 0x%08X", hr);
        return hr;
    }

    return GetLongProperty(L"Count", lCount);
}

HRESULT CatalogCollection::GetName(BSTR& bstrName)
{
    HRESULT hr = Populate();
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCount: Failed to populate Applications collection. HRESULT: 0x%08X", hr);
        return hr;
    }

    return GetStringProperty(L"Name", bstrName);
}