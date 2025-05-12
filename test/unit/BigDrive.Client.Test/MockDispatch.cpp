// <copyright file="ApplicationCollectionTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <oaidl.h>
#include <iostream>

class MockDispatch : public IDispatch 
{
public:

    // Reference count for COM object
    ULONG refCount = 1;

    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override 
    {
        if (riid == IID_IUnknown || riid == IID_IDispatch) {
            *ppvObject = static_cast<IDispatch*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override 
    {
        return ++refCount;
    }

    ULONG STDMETHODCALLTYPE Release() override 
    {
        if (--refCount == 0) {
            delete this;
            return 0;
        }
        return refCount;
    }

    // IDispatch methods
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override 
    {
        *pctinfo = 0; // No type info available
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override 
    {
        return E_NOTIMPL; // Not implemented
    }

    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override 
    {
        if (cNames == 1 && wcscmp(rgszNames[0], L"MockMethod") == 0) {
            rgDispId[0] = 1; // Assign a mock DISPID
            return S_OK;
        }
        return DISP_E_UNKNOWNNAME;
    }

    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
        DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override 
    {
        if (dispIdMember == 1) {
            std::cout << "MockMethod invoked!" << std::endl;
            if (pVarResult) {
                VariantInit(pVarResult);
                pVarResult->vt = VT_BSTR;
                pVarResult->bstrVal = SysAllocString(L"Mock Response");
            }
            return S_OK;
        }
        return DISP_E_MEMBERNOTFOUND;
    }
};