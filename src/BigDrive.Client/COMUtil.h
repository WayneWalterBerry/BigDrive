// <copyright file="COMUtil.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <comutil.h>

inline HRESULT CreateSafeArrayFromBSTR(BSTR bstr, SAFEARRAY** ppSafeArray)
{
    if (!bstr || !ppSafeArray)
        return E_POINTER;

    *ppSafeArray = nullptr;

    SAFEARRAYBOUND sabound;
    sabound.lLbound = 0;
    sabound.cElements = 1;

    SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, &sabound);
    if (!psa)
        return E_OUTOFMEMORY;

    VARIANT var;
    VariantInit(&var);
    var.vt = VT_BSTR;
    var.bstrVal = bstr;

    LONG index = 0;
    HRESULT hr = SafeArrayPutElement(psa, &index, &var);

    if (SUCCEEDED(hr))
    {
        *ppSafeArray = psa;
    }
    else
    {
        SafeArrayDestroy(psa);
    }

    // Do not clear var, as it does not own the BSTR
    return hr;
}
