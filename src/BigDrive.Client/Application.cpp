// <copyright file="Application.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comadmin.h>
#include <oaidl.h>

// Header
#include "Application.h"

// Local
#include "Component.h"

HRESULT Application::GetComponentCollection(ComponentCollection** ppComponentCollection)
{
    HRESULT hrReturn = S_OK;

    if (!ppComponentCollection)
    {
        s_eventLogger.WriteError(L"GetComponents: Invalid pointer passed for ppComponentCollection.");
        return E_POINTER;
    }

    DISPID dispidGetCollection;
    const OLECHAR* methodName = L"GetCollection";

    VARIANT vtCollections;
    ::VariantInit(&vtCollections);
    DISPPARAMS params = { nullptr, nullptr, 0, 0 };

    VARIANTARG varg;
    varg.vt = VT_BSTR;
    varg.bstrVal = ::SysAllocString(L"Components"); // Request Collection list
    params.rgvarg = &varg;
    params.cArgs = 1;

    // Get the DISPID for GetCollection method
    hrReturn = GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&methodName), 1, LOCALE_USER_DEFAULT, &dispidGetCollection);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetComponentCollection: Failed to get DISPID for GetCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = Invoke(dispidGetCollection, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &vtCollections, nullptr, nullptr);
    if (FAILED(hrReturn) || (vtCollections.vt != VT_DISPATCH))
    {
        s_eventLogger.WriteErrorFormmated(L"GetComponentCollection: Failed to invoke GetCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    *ppComponentCollection = new ComponentCollection(m_pCOMAdminCatalog, vtCollections.pdispVal);

End:

    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetComponentCollection: Operation failed. HRESULT: 0x%08X", hrReturn);
    }


    if (varg.bstrVal)
    {
        ::SysFreeString(varg.bstrVal);
    }

    return hrReturn;
}
