// ICatalogCollection.h
// C++ interface definition extracted from ComAdmin.h

#pragma once

#include <windows.h>
#include <unknwn.h>
#include <oaidl.h>

// {6eb22872-8a19-11d0-81b6-00a0c9231c29}
DEFINE_GUID(IID_ICatalogCollection,
    0x6eb22872, 0x8a19, 0x11d0, 0x81, 0xb6, 0x00, 0xa0, 0xc9, 0x23, 0x1c, 0x29);

#ifdef __cplusplus

// C++ interface definition for ICatalogCollection
#undef INTERFACE
#define INTERFACE ICatalogCollection

DECLARE_INTERFACE_(ICatalogCollection, IDispatch)
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

    // ICatalogCollection methods
    STDMETHOD(get__NewEnum)(THIS_ IUnknown * *ppEnumVariant) PURE;
    STDMETHOD(get_Item)(THIS_ long lIndex, IDispatch * *ppCatalogObject) PURE;
    STDMETHOD(get_Count)(THIS_ long* plObjectCount) PURE;
    STDMETHOD(Remove)(THIS_ long lIndex) PURE;
    STDMETHOD(Add)(THIS_ IDispatch * *ppCatalogObject) PURE;
    STDMETHOD(Populate)(THIS) PURE;
    STDMETHOD(SaveChanges)(THIS_ long* pcChanges) PURE;
    STDMETHOD(GetCollection)(THIS_ BSTR bstrCollName, VARIANT varObjectKey, IDispatch * *ppCatalogCollection) PURE;
    STDMETHOD(get_Name)(THIS_ VARIANT * pVarNamel) PURE;
    STDMETHOD(get_AddEnabled)(THIS_ VARIANT_BOOL * pVarBool) PURE;
    STDMETHOD(get_RemoveEnabled)(THIS_ VARIANT_BOOL * pVarBool) PURE;
    STDMETHOD(GetUtilInterface)(THIS_ IDispatch * *ppIDispatch) PURE;
    STDMETHOD(get_DataStoreMajorVersion)(THIS_ long* plMajorVersion) PURE;
    STDMETHOD(get_DataStoreMinorVersion)(THIS_ long* plMinorVersionl) PURE;
    STDMETHOD(PopulateByKey)(THIS_ SAFEARRAY * psaKeys) PURE;
    STDMETHOD(PopulateByQuery)(THIS_ BSTR bstrQueryString, long lQueryType) PURE;
};

#endif // __cplusplus
