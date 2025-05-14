// ICatalogObject.h
// C++ interface definition extracted from ComAdmin.h

#pragma once

#include <windows.h>
#include <unknwn.h>
#include <oaidl.h>

// {6eb22871-8a19-11d0-81b6-00a0c9231c29}
DEFINE_GUID(IID_ICatalogObject,
    0x6eb22871, 0x8a19, 0x11d0, 0x81, 0xb6, 0x00, 0xa0, 0xc9, 0x23, 0x1c, 0x29);

#ifdef __cplusplus

#undef INTERFACE
#define INTERFACE ICatalogObject

DECLARE_INTERFACE_(ICatalogObject, IDispatch)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // IDispatch methods
    STDMETHOD(GetTypeInfoCount)(THIS_ UINT * pctinfo) PURE;
    STDMETHOD(GetTypeInfo)(THIS_ UINT iTInfo, LCID lcid, ITypeInfo * *ppTInfo) PURE;
    STDMETHOD(GetIDsOfNames)(THIS_ REFIID riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId) PURE;
    STDMETHOD(Invoke)(THIS_ DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pDispParams, VARIANT * pVarResult, EXCEPINFO * pExcepInfo, UINT * puArgErr) PURE;

    // ICatalogObject methods
    STDMETHOD(get_Value)(THIS_ BSTR bstrPropName, VARIANT * pvarRetVal) PURE;
    STDMETHOD(put_Value)(THIS_ BSTR bstrPropName, VARIANT val) PURE;
    STDMETHOD(get_Key)(THIS_ VARIANT * pvarRetVal) PURE;
    STDMETHOD(get_Name)(THIS_ VARIANT * pvarRetVal) PURE;
    STDMETHOD(IsPropertyReadOnly)(THIS_ BSTR bstrPropName, VARIANT_BOOL * pbRetVal) PURE;
    STDMETHOD(get_Valid)(THIS_ VARIANT_BOOL * pbRetVal) PURE;
    STDMETHOD(IsPropertyWriteOnly)(THIS_ BSTR bstrPropName, VARIANT_BOOL * pbRetVal) PURE;
};

#endif // __cplusplus
