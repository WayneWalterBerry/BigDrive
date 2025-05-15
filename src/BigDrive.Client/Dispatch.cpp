// <copyright file="Dispatch.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comdef.h>

// Header
#include "Dispatch.h"

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "FuncDesc.h"

// Initialize the static EventLogger instance
EventLogger Dispatch::s_eventLogger(L"BigDrive.Client");

// Define the static member outside the class
DISPPARAMS Dispatch::dispparamsNoArgs = { nullptr, nullptr, 0, 0 };

HRESULT Dispatch::GetProperty(LPCWSTR szName, VARIANT* pValue)
{
    HRESULT hrReturn = S_OK;

    if (!szName || !pValue)
    {
        return E_POINTER;
    }

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    DISPID dispid;

    ::VariantInit(pValue);

    // Get the property name
    LPOLESTR pOleStrName = ::SysAllocString(szName);

    hrReturn = GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&pOleStrName), 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hrReturn))
    {
        goto End;
    }

    hrReturn = GetProperty(dispid, pValue);
    if (FAILED(hrReturn))
    {
        goto End;
    }

End:

    if (pOleStrName)
    {
        ::SysFreeString(pOleStrName);
        pOleStrName = nullptr;
    }

    return hrReturn;
}

HRESULT Dispatch::GetProperty(LPCWSTR szName, LPDISPATCH* pIDispatch)
{
    HRESULT hr = S_OK;

    if (!szName || !pIDispatch)
    {
        return E_POINTER;
    }

    VARIANT vtResult;

    ::VariantInit(&vtResult);

    hr = GetProperty(szName, &vtResult);
    if (FAILED(hr) || (vtResult.vt != VT_DISPATCH))
    {
        goto End;
    }

    *pIDispatch = vtResult.pdispVal;
    (*pIDispatch)->AddRef();

End:

    ::VariantClear(&vtResult);

    return hr;
}

HRESULT Dispatch::GetProperty(DISPID dispid, VARIANT* pValue)
{
    HRESULT hrReturn = S_OK;

    if (!pValue)
    {
        return E_POINTER;
    }

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    VARIANT vtResult;

    ::VariantInit(&vtResult);

    // Invoke the property
    hrReturn = m_pIDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &vtResult, nullptr, nullptr);
    if (FAILED(hrReturn))
    {
        goto End;
    }

    // Copy the result to the output parameter
    *pValue = vtResult;

End:

    return hrReturn;
}

HRESULT Dispatch::GetStringProperty(LPCWSTR szName, BSTR& bstrString)
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
    if (vtValue.vt != VT_BSTR)
    {
        hrReturn = E_FAIL;
        goto End;
    }

    bstrString = ::SysAllocString(vtValue.bstrVal);
    if (bstrString == nullptr)
    {
        hrReturn = E_OUTOFMEMORY;
    }

End:

    ::VariantClear(&vtValue);

    return hrReturn;
}

HRESULT Dispatch::GetProperty(LPCWSTR szName, CLSID& clsid)
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
    if (vtValue.vt != VT_BSTR)
    {
        hrReturn = E_FAIL;
        goto End;
    }

    hrReturn = ::CLSIDFromString(vtValue.bstrVal, &clsid);
    if (FAILED(hrReturn))
    {
        goto End;
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

HRESULT Dispatch::GetNames(BSTR** ppNames, LONG& lCount)
{
    if (!ppNames)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    ITypeInfo* pTypeInfo = nullptr;
    TYPEATTR* pTypeAttr = nullptr;
    UINT count = 0;

    *ppNames = nullptr;
    lCount = 0;

    // Get type information
    hr = m_pIDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
    if (FAILED(hr) || !pTypeInfo)
    {
        goto End;
    }

    // Get type attributes
    hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
    if (FAILED(hr) || !pTypeAttr)
    {
        goto End;
    }

    // Allocate memory for names
    *ppNames = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * pTypeAttr->cFuncs);
    if (!*ppNames)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Retrieve method names
    for (UINT i = 0; i < pTypeAttr->cFuncs; i++)
    {
        FuncDesc* pFuncDesc = nullptr;

        hr = FuncDesc::Create(pTypeInfo, i, &pFuncDesc);

        BSTR bstrJson;
        HRESULT hr = pFuncDesc->Serialize(bstrJson);
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"GetNames: Failed to serialize FuncDesc. HRESULT: 0x%08X", hr);

            delete pFuncDesc;
            pFuncDesc = nullptr;

            goto End;
        }

        (*ppNames)[count++] = bstrJson;

        if (pFuncDesc)
        {
            delete pFuncDesc;
            pFuncDesc = nullptr;
        }
    }

    lCount = count;

End:

    // Cleanup
    if (pTypeInfo != nullptr)
    {
        pTypeInfo->Release();
        pTypeInfo = nullptr;
    }

    if (pTypeInfo != nullptr)
    {
        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
        pTypeAttr = nullptr;
    }

    return hr;
}

HRESULT Dispatch::GetSupportedIIDs(IID** pResult, ULONG& ulCount)
{
    HRESULT hr = S_OK;

    if (!pResult)
    {
        return E_POINTER;
    }

    *pResult = nullptr;
    ulCount = 0;

    ITypeInfo* pTypeInfo = nullptr;
    hr = m_pIDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
    if (FAILED(hr))
    {
        return hr; // Failed to get ITypeInfo
    }

    TYPEATTR* pTypeAttr = nullptr;
    hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
    if (FAILED(hr))
    {
        pTypeInfo->Release();
        return hr; // Failed to get TYPEATTR
    }

    // Allocate memory for the array of IIDs
    ulCount = pTypeAttr->cImplTypes;
    *pResult = (IID*)CoTaskMemAlloc(sizeof(IID) * ulCount);
    if (!*pResult)
    {
        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
        pTypeInfo->Release();
        return E_OUTOFMEMORY;
    }

    // Retrieve the IIDs of implemented interfaces
    for (UINT i = 0; i < pTypeAttr->cImplTypes; i++)
    {
        HREFTYPE hRefType;
        hr = pTypeInfo->GetRefTypeOfImplType(i, &hRefType);
        if (FAILED(hr))
        {
            CoTaskMemFree(*pResult);
            *pResult = nullptr;
            ulCount = 0;
            pTypeInfo->ReleaseTypeAttr(pTypeAttr);
            pTypeInfo->Release();
            return hr;
        }

        ITypeInfo* pRefTypeInfo = nullptr;
        hr = pTypeInfo->GetRefTypeInfo(hRefType, &pRefTypeInfo);
        if (FAILED(hr))
        {
            CoTaskMemFree(*pResult);
            *pResult = nullptr;
            ulCount = 0;
            pTypeInfo->ReleaseTypeAttr(pTypeAttr);
            pTypeInfo->Release();
            return hr;
        }

        TYPEATTR* pRefTypeAttr = nullptr;
        hr = pRefTypeInfo->GetTypeAttr(&pRefTypeAttr);
        if (FAILED(hr))
        {
            pRefTypeInfo->Release();
            CoTaskMemFree(*pResult);
            *pResult = nullptr;
            ulCount = 0;
            pTypeInfo->ReleaseTypeAttr(pTypeAttr);
            pTypeInfo->Release();
            return hr;
        }

        // Copy the IID to the result array
        (*pResult)[i] = pRefTypeAttr->guid;

        pRefTypeInfo->ReleaseTypeAttr(pRefTypeAttr);
        pRefTypeInfo->Release();
    }

    // Cleanup
    pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    pTypeInfo->Release();

    return S_OK;
}

HRESULT Dispatch::GetValue(LPCWSTR szName, BSTR& bstrValue)
{
    VARIANT varResult;
    ::VariantInit(&varResult);

    VARIANT varPropertyName;
    ::VariantInit(&varPropertyName);
    varPropertyName.vt = VT_BSTR;
    varPropertyName.bstrVal = SysAllocString(szName);

    if (!varPropertyName.bstrVal)
    {
        s_eventLogger.WriteError(L"GetValue: Failed to allocate memory for property name.");
        return E_OUTOFMEMORY;
    }

    DISPID dispidGetValue;
    LPOLESTR szMethodName = ::SysAllocString(L"Value");

    if (!szMethodName)
    {
        s_eventLogger.WriteError(L"GetValue: Failed to allocate memory for method name.");
        ::SysFreeString(varPropertyName.bstrVal);
        return E_OUTOFMEMORY;
    }

    DISPPARAMS params = { &varPropertyName, nullptr, 1, 0 };

    HRESULT hr = GetIDsOfNames(IID_NULL, &szMethodName, 1, LOCALE_USER_DEFAULT, &dispidGetValue);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetValue: Failed to get DISPID for method 'Value'. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = Invoke(dispidGetValue, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &varResult, nullptr, nullptr);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetValue: Failed to invoke property 'Value'. HRESULT: 0x%08X", hr);
        goto End;
    }

    if (varResult.vt != VT_BSTR)
    {
        s_eventLogger.WriteError(L"GetValue: Returned value is not of type BSTR.");
        hr = E_FAIL;
        goto End;
    }

    bstrValue = ::SysAllocString(varResult.bstrVal);
    if (!bstrValue)
    {
        s_eventLogger.WriteError(L"GetValue: Failed to allocate memory for the result BSTR.");
        hr = E_OUTOFMEMORY;
        goto End;
    }

End:

    ::VariantClear(&varResult);

    if (szMethodName)
    {
        ::SysFreeString(szMethodName);
        szMethodName = nullptr;
    }

    if (varPropertyName.bstrVal)
    {
        ::SysFreeString(varPropertyName.bstrVal);
        varPropertyName.bstrVal = nullptr;
    }

    return hr;
}

HRESULT Dispatch::GetValue(LPCWSTR szName, CLSID& clsid)
{
    HRESULT hr = S_OK;
    BSTR bstrValue;

    hr = GetValue(szName, bstrValue);
    if (FAILED(hr))
    {
        goto End;
    }

    hr = ::CLSIDFromString(bstrValue, &clsid);
    if (FAILED(hr))
    {
        goto End;
    }

End:

    if (bstrValue)
    {
        ::SysFreeString(bstrValue);
        bstrValue = nullptr;
    }
    return hr;
}

HRESULT Dispatch::GetTypeInfo(BSTR& bstrName)
{
    HRESULT hr = S_OK;
    UINT index;

    ITypeInfo* pTypeInfo = nullptr;
    ITypeLib* pTypeLib = nullptr;

    hr = m_pIDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
    if (FAILED(hr) || !pTypeInfo)
    {
        goto End;
    }

    hr = pTypeInfo->GetContainingTypeLib(&pTypeLib, &index);
    if (FAILED(hr) || !pTypeLib)
    {
        goto End;
    }

    // Get Type Library Name
    hr = pTypeLib->GetDocumentation(-1, &bstrName, nullptr, nullptr, nullptr);
    if (FAILED(hr))
    {
        goto End;
    }

End:

    if (pTypeLib != nullptr)
    {
        pTypeLib->Release();
        pTypeLib = nullptr;
    }

    if (pTypeInfo != nullptr)
    {
        pTypeInfo->Release();
        pTypeInfo = nullptr;
    }

    return hr;
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

    HRESULT hr = m_pIDispatch->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
    switch (hr)
    {
    case DISP_E_UNKNOWNNAME:

        BSTR bstrFuncJson;

        HRESULT hrInternal = FunctionDescriptions(bstrFuncJson);
        if (FAILED(hrInternal))
        {
            ::SysFreeString(bstrFuncJson);
            s_eventLogger.WriteErrorFormmated(L"FunctionDescriptions: Failed to get names. HRESULT: 0x%08X", hrInternal);
            goto End;
        }

        s_eventLogger.WriteErrorFormmated(L"Invoke: Unknown name. HRESULT: 0x%08X Possible Names: {%s}", hr, bstrFuncJson);

        ::SysFreeString(bstrFuncJson);

        break;
    }

End:

    return hr;
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
    HRESULT hrInternal;
    FuncDesc* pFuncDesc = nullptr;
    BSTR bstrFuncJson;

    if (m_pIDispatch == nullptr)
    {
        return E_POINTER;
    }

    HRESULT hr = m_pIDispatch->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
    switch (hr)
    {
    case DISP_E_TYPEMISMATCH:

        hrInternal = GetFuncDesc(dispIdMember, &pFuncDesc);
        if (FAILED(hrInternal))
        {
            s_eventLogger.WriteErrorFormmated(L"FunctionDescriptions: GetFuncDesc() failed. HRESULT: 0x%08X", hrInternal);
            goto End;
        }

        hrInternal = pFuncDesc->Serialize(bstrFuncJson);
        if (FAILED(hrInternal))
        {
            s_eventLogger.WriteErrorFormmated(L"FunctionDescriptions: Serialize() failed. HRESULT: 0x%08X", hrInternal);
            goto End;
        }

        s_eventLogger.WriteErrorFormmated(L"Invoke: Type Mistmatch. HRESULT: 0x%08X Possible Names: {%s}", hr, bstrFuncJson);

        ::SysFreeString(bstrFuncJson);

        break;
    case DISP_E_MEMBERNOTFOUND:

        hrInternal = FunctionDescriptions(bstrFuncJson);
        if (FAILED(hrInternal))
        {
            ::SysFreeString(bstrFuncJson);
            s_eventLogger.WriteErrorFormmated(L"FunctionDescriptions: Failed to function Descriptions. HRESULT: 0x%08X", hrInternal);
            goto End;
        }

        s_eventLogger.WriteErrorFormmated(L"Invoke: Unknown name. HRESULT: 0x%08X Possible Names: {%s}", hr, bstrFuncJson);

        ::SysFreeString(bstrFuncJson);

        break;
    }

End:

    return hr;
}

HRESULT Dispatch::CreateJsonArray(BSTR* pJson, LONG lCount, BSTR& bstrJsonArray)
{
    if (!pJson || lCount <= 0)
    {
        return E_INVALIDARG; // Invalid arguments
    }

    try
    {
        std::ostringstream jsonStream;

        // Start JSON array
        jsonStream << "[";

        for (LONG i = 0; i < lCount; ++i)
        {
            if (pJson[i] == nullptr)
            {
                return E_POINTER; // Null pointer in the array
            }

            // Append the JSON object to the array
            jsonStream << _bstr_t(pJson[i]);

            // Add a comma if it's not the last element
            if (i < lCount - 1)
            {
                jsonStream << ",";
            }
        }

        // End JSON array
        jsonStream << "]";

        // Convert JSON string to BSTR
        std::string jsonString = jsonStream.str();
        _bstr_t bstr(jsonString.c_str());
        bstrJsonArray = bstr.Detach();

        return S_OK;
    }
    catch (const std::exception&)
    {
        return E_FAIL; // Return failure in case of an exception
    }
}

HRESULT Dispatch::FunctionDescriptions(BSTR& bstrJson)
{
    HRESULT hr;

    BSTR* pNames = nullptr;
    LONG lCount = 0;

    hr = GetNames(&pNames, lCount);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetIDsOfNames: Failed to get names. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = CreateJsonArray(pNames, lCount, bstrJson);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetIDsOfNames: Failed to create json array. HRESULT: 0x%08X", hr);
        goto End;
    }

End:

    // Cleanup
    if (pNames)
    {
        for (LONG i = 0; i < lCount; ++i)
        {
            if (pNames[i])
            {
                ::SysFreeString(pNames[i]);
                pNames[i] = nullptr;
            }
        }
        ::CoTaskMemFree(pNames);
        pNames = nullptr;
    }

    return hr;
}

HRESULT Dispatch::GetFuncDesc(LPFUNCDESC** pppFuncDesc, LONG& lCount)
{
    if (!pppFuncDesc)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    ITypeInfo* pTypeInfo = nullptr;
    TYPEATTR* pTypeAttr = nullptr;
    UINT count = 0;

    *pppFuncDesc = nullptr;
    lCount = 0;

    // Get type information
    hr = m_pIDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
    if (FAILED(hr) || !pTypeInfo)
    {
        goto End;
    }

    // Get type attributes
    hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
    if (FAILED(hr) || !pTypeAttr)
    {
        goto End;
    }

    // Allocate memory for names
    *pppFuncDesc = (LPFUNCDESC*)::CoTaskMemAlloc(sizeof(LPFUNCDESC) * pTypeAttr->cFuncs);
    if (!*pppFuncDesc)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Retrieve method names
    for (UINT i = 0; i < pTypeAttr->cFuncs; i++)
    {
        FuncDesc* pFuncDesc = nullptr;

        hr = FuncDesc::Create(pTypeInfo, i, &pFuncDesc);
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"GetFuncDesc: Failed to create FuncDesc. HRESULT: 0x%08X", hr);
            goto End;
        }

        (*pppFuncDesc)[count++] = pFuncDesc;
    }

    lCount = count;

End:

    // Cleanup
    if (pTypeInfo != nullptr)
    {
        pTypeInfo->Release();
        pTypeInfo = nullptr;
    }

    if (pTypeInfo != nullptr)
    {
        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
        pTypeAttr = nullptr;
    }

    return hr;
}

HRESULT Dispatch::GetFuncDesc(DISPID dispid, LPFUNCDESC* ppFuncDesc)
{
    HRESULT hr = S_OK;

    LPFUNCDESC* arrayFuncDesc = nullptr;
    LONG lCount = 0;

    hr = GetFuncDesc(&arrayFuncDesc, lCount);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetFuncDesc: Failed to get FuncDesc. HRESULT: 0x%08X", hr);
        goto End;
    }

    for (LONG i = 0; i < lCount; i++)
    {
        if (arrayFuncDesc[i]->GetMemId() == dispid)
        {
            hr = arrayFuncDesc[i]->Clone(ppFuncDesc);
            if (FAILED(hr))
            {
                s_eventLogger.WriteErrorFormmated(L"GetFuncDesc: Failed to get FuncDesc. HRESULT: 0x%08X", hr);
                goto End;
            }

            break;
        }
    }

End:

    // Cleanup
    if (arrayFuncDesc)
    {
        for (LONG i = 0; i < lCount; i++)
        {
            if (arrayFuncDesc[i])
            {
                delete arrayFuncDesc[i];
                arrayFuncDesc[i] = nullptr;
            }
        }
        ::CoTaskMemFree(arrayFuncDesc);
        arrayFuncDesc = nullptr;
    }

    return hr;
}

