// <copyright file="COM.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comdef.h>

// Header
#include "COM.h"

HRESULT COM::GetProperty(IDispatch* pIDispatch, LPCWSTR szName, VARIANT* pValue)
{
    HRESULT hrReturn = S_OK;

    if (!pIDispatch || !szName || !pValue)
    {
        return E_POINTER;
    }

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    VARIANT vtResult;
    DISPID dispid;

    ::VariantInit(&vtResult);

    // Get the property name
    LPOLESTR pOleStrName = ::SysAllocString(szName);

    hrReturn = pIDispatch->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&pOleStrName), 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hrReturn))
    {
        goto End;
    }
    // Invoke the property
    hrReturn = pIDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &vtResult, nullptr, nullptr);
    if (FAILED(hrReturn))
    {
        goto End;
    }

    // Copy the result to the output parameter
    *pValue = vtResult;

End:

    if (pOleStrName)
    {
        ::SysFreeString(pOleStrName);
        pOleStrName = nullptr;
    }

    return hrReturn;
}

HRESULT COM::GetStringProperty(IDispatch* pIDispatch, LPCWSTR szName, BSTR& bstrString)
{
    HRESULT hrReturn = S_OK;
    VARIANT vtValue;

    if (!pIDispatch || !szName || !bstrString)
    {
        return E_POINTER;
    }

    ::VariantInit(&vtValue);

    // Get the property value
    hrReturn = GetProperty(pIDispatch, szName, &vtValue);
    if (FAILED(hrReturn))
    {
        goto End;
    }
    // Check if the property is a string
    if (vtValue.vt == VT_BSTR)
    {
        bstrString = ::SysAllocString(vtValue.bstrVal);
        if (bstrString == nullptr)
        {
            hrReturn = E_OUTOFMEMORY;
        }
    }
    else
    {
        hrReturn = E_FAIL; // Not a string type
    }
End:

    ::VariantClear(&vtValue);

    return hrReturn;
}

HRESULT COM::GetLongProperty(IDispatch* pIDispatch, LPCWSTR szName, LONG& value)
{
    HRESULT hrReturn = S_OK;
    VARIANT vtValue;

    if (!pIDispatch || !szName)
    {
        return E_POINTER;
    }

    ::VariantInit(&vtValue);

    // Get the property value
    hrReturn = GetProperty(pIDispatch, szName, &vtValue);
    if (FAILED(hrReturn))
    {
        goto End;
    }
    // Check if the property is a string
    if (vtValue.vt != VT_I4)
    {
        hrReturn = E_FAIL; 
        goto End;
    }

    value = vtValue.lVal;

End:

    ::VariantClear(&vtValue);

    return hrReturn;
}

