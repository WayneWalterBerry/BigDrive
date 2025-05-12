// <copyright file="Dispatch.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comdef.h>

// Header
#include "Dispatch.h"

HRESULT Dispatch::GetProperty(LPCWSTR szName, VARIANT* pValue)
{
    HRESULT hrReturn = S_OK;

    if (!szName || !pValue)
    {
        return E_POINTER;
    }

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    VARIANT vtResult;
    DISPID dispid;

    ::VariantInit(&vtResult);

    // Get the property name
    LPOLESTR pOleStrName = ::SysAllocString(szName);

    hrReturn = m_pIDispatch->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&pOleStrName), 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hrReturn))
    {
        goto End;
    }
    // Invoke the property
    hrReturn = m_pIDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &vtResult, nullptr, nullptr);
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

HRESULT Dispatch::GetStringProperty(LPCWSTR szName, BSTR& bstrString)
{
    HRESULT hrReturn = S_OK;
    VARIANT vtValue;

    if (!szName || !bstrString)
    {
        return E_POINTER;
    }

    ::VariantInit(&vtValue);

    // Get the property value
    hrReturn = GetProperty(szName, &vtValue);
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

HRESULT Dispatch::GetLongProperty(LPCWSTR szName, LONG& value)
{
    HRESULT hrReturn = S_OK;
    VARIANT vtValue;

    if (!szName)
    {
        return E_POINTER;
    }

    ::VariantInit(&vtValue);

    // Get the property value
    hrReturn = GetProperty(szName, &vtValue);
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

/// <summary>
/// Increments the reference count for the object.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG Dispatch::AddRef()
{
    if (m_pIDispatch == nullptr)
    {
        return 0;
    }
    return m_pIDispatch->AddRef();
}

/// <summary>
/// Decrements the reference count for the object.
/// </summary>
/// <returns>The new reference count.</returns>
ULONG Dispatch::Release()
{
    if (m_pIDispatch == nullptr)
    {
        return 0;
    }
    ULONG refCount = m_pIDispatch->Release();
    if (refCount == 0)
    {
        delete this;
    }
    return refCount;
}

/// <summary>
/// Retrieves pointers to the supported interfaces on an object.
/// </summary>
/// <param name="riid">The identifier of the interface being requested.</param>
/// <param name="ppvObject">Address of a pointer variable that receives the interface pointer requested.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT Dispatch::QueryInterface(REFIID riid, void** ppvObject)
{
    if (ppvObject == nullptr)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown || riid == IID_IDispatch)
    {
        *ppvObject = static_cast<IDispatch*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

/// <summary>
/// Retrieves the type information count.
/// </summary>
/// <param name="pctinfo">Pointer to receive the type information count.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT Dispatch::GetTypeInfoCount(UINT* pctinfo)
{
    if (m_pIDispatch == nullptr)
    {
        return E_POINTER;
    }
    return m_pIDispatch->GetTypeInfoCount(pctinfo);
}

/// <summary>
/// Retrieves the type information for the object.
/// </summary>
/// <param name="iTInfo">The type information to return.</param>
/// <param name="lcid">The locale identifier for the type information.</param>
/// <param name="ppTInfo">Pointer to receive the type information.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT Dispatch::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    if (m_pIDispatch == nullptr)
    {
        return E_POINTER;
    }
    return m_pIDispatch->GetTypeInfo(iTInfo, lcid, ppTInfo);
}

/// <summary>
/// Maps a single member and an optional set of argument names to a corresponding set of DISPIDs.
/// </summary>
/// <param name="riid">Reserved for future use. Must be IID_NULL.</param>
/// <param name="rgszNames">Array of names to be mapped.</param>
/// <param name="cNames">The count of the names to be mapped.</param>
/// <param name="lcid">The locale context in which to interpret the names.</param>
/// <param name="rgDispId">Caller-allocated array to receive the DISPIDs.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT Dispatch::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
    if (m_pIDispatch == nullptr)
    {
        return E_POINTER;
    }
    return m_pIDispatch->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
}

/// <summary>
/// Provides access to properties and methods exposed by an object.
/// </summary>
/// <param name="dispIdMember">Identifies the member.</param>
/// <param name="riid">Reserved for future use. Must be IID_NULL.</param>
/// <param name="lcid">The locale context in which to interpret arguments.</param>
/// <param name="wFlags">Flags describing the context of the call.</param>
/// <param name="pDispParams">Pointer to the arguments passed to the method or property.</param>
/// <param name="pVarResult">Pointer to the result of the call.</param>
/// <param name="pExcepInfo">Pointer to exception information.</param>
/// <param name="puArgErr">The index of the first argument with an error.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT Dispatch::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    if (m_pIDispatch == nullptr)
    {
        return E_POINTER;
    }
    return m_pIDispatch->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

