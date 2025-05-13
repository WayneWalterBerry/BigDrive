// <copyright file="Dispatch.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Shared
#include "..\Shared\EventLogger.h"

// Local
#include "FuncDesc.h"

class Dispatch : public IDispatch
{
protected:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

    static DISPPARAMS dispparamsNoArgs;

    LPDISPATCH m_pIDispatch;

public:

    Dispatch(LPDISPATCH pIDispatch)
        : m_pIDispatch(pIDispatch)
    {
        if (m_pIDispatch == nullptr)
        {
            throw E_POINTER;
        }

        if (m_pIDispatch)
        {
            m_pIDispatch->AddRef();
        }
    }

    ~Dispatch()
    {
        if (m_pIDispatch)
        {
            m_pIDispatch->Release();
            m_pIDispatch = nullptr;
        }
    }

    /// <summary>
    /// Retrieves a property value by name.
    /// </summary>
    /// <param name="szName">The name of the property.</param>
    /// <param name="pValue">Pointer to a VARIANT to receive the property value.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetProperty(LPCWSTR szName, VARIANT* pValue);

    HRESULT GetProperty(DISPID dispid, VARIANT* pValue);

    /// <summary>
    /// Retrieves a string property value by name.
    /// </summary>
    /// <param name="szName">The name of the property.</param>
    /// <param name="bstrString">Reference to a BSTR to receive the property value.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetStringProperty(LPCWSTR szName, BSTR& bstrString);

    /// <summary>
    /// Retrieves a long integer property value by name.
    /// </summary>
    /// <param name="szName">The name of the property.</param>
    /// <param name="value">Reference to a LONG to receive the property value.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetLongProperty(LPCWSTR szName, LONG& value);

    HRESULT GetProperty(LPCWSTR szName, LPDISPATCH* pIDispatch);

    HRESULT GetNames(BSTR** ppNames, LONG& lCount);

    /// <summary>
    /// Call the Value Property on with the Value Name As the Argument To The Value Property
    /// </summary>
    /// <param name="szName">Name of Value to fetch</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetValue(LPCWSTR bstrName, BSTR& bstrValue);

    HRESULT GetTypeInfo(BSTR& bstrName);

    // ==================== IUnknown Methods ====================

    /// <summary>
    /// Increments the reference count for the object.
    /// </summary>
    /// <returns>The new reference count.</returns>
    ULONG AddRef() override;

    /// <summary>
    /// Decrements the reference count for the object.
    /// </summary>
    /// <returns>The new reference count.</returns>
    ULONG Release() override;

    /// <summary>
    /// Retrieves pointers to the supported interfaces on an object.
    /// </summary>
    /// <param name="riid">The identifier of the interface being requested.</param>
    /// <param name="ppvObject">Address of a pointer variable that receives the interface pointer requested.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT QueryInterface(REFIID riid, void** ppvObject) override;

    // ==================== IDispatch Methods ====================

    /// <summary>
    /// Retrieves the type information count.
    /// </summary>
    /// <param name="pctinfo">Pointer to receive the type information count.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetTypeInfoCount(UINT* pctinfo) override;

    /// <summary>
    /// Retrieves the type information for the object.
    /// </summary>
    /// <param name="iTInfo">The type information to return.</param>
    /// <param name="lcid">The locale identifier for the type information.</param>
    /// <param name="ppTInfo">Pointer to receive the type information.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;

    /// <summary>
    /// Maps a single member and an optional set of argument names to a corresponding set of DISPIDs.
    /// </summary>
    /// <param name="riid">Reserved for future use. Must be IID_NULL.</param>
    /// <param name="rgszNames">Array of names to be mapped.</param>
    /// <param name="cNames">The count of the names to be mapped.</param>
    /// <param name="lcid">The locale context in which to interpret the names.</param>
    /// <param name="rgDispId">Caller-allocated array to receive the DISPIDs.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override;

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
    HRESULT Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

    HRESULT CreateJsonArray(BSTR* pJson, LONG lCount, BSTR& bstrJsonArray);

    HRESULT FunctionDescriptions(BSTR& bstrJson);

    HRESULT GetFuncDesc(FuncDesc*** pppFuncDesc, LONG& lCount);

    HRESULT GetFuncDesc(DISPID dispid, LPFUNCDESC*ppFuncDesc);

private:

    HRESULT GetSupportedIIDs(IID** pResult, ULONG& ulCount);
};