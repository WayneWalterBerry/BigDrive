#pragma once

#include <windows.h>
#include <guiddef.h>

// ICatalogCollection interface definition - C-style declaration

#ifndef __ICatalogCollection_INTERFACE_DEFINED__
#define __ICatalogCollection_INTERFACE_DEFINED__

// Add this before the interface definition
// This is the actual GUID for ICatalogCollection from ComAdmin.h
DEFINE_GUID(IID_ICatalogCollection,
    0x6eb22872, 0x8a19, 0x11d0, 0x81, 0xb6, 0x00, 0xa0, 0xc9, 0x23, 0x1c, 0x29);

// Forward declarations
typedef interface ICatalogCollection ICatalogCollection;

// UUID for ICatalogCollection interface
EXTERN_C const IID IID_ICatalogCollection;

// C-style interface definition
typedef struct ICatalogCollectionVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            __RPC__in ICatalogCollection* This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in ICatalogCollection* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in ICatalogCollection* This);

    HRESULT(STDMETHODCALLTYPE* GetTypeInfoCount)(
        __RPC__in ICatalogCollection* This,
        /* [out] */ __RPC__out UINT* pctinfo);

    HRESULT(STDMETHODCALLTYPE* GetTypeInfo)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ __RPC__deref_out_opt ITypeInfo** ppTInfo);

    HRESULT(STDMETHODCALLTYPE* GetIDsOfNames)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR* rgszNames,
        /* [range][in] */ __RPC__in_range(0, 16384) UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID* rgDispId);

    /* [local] */ HRESULT(STDMETHODCALLTYPE* Invoke)(
        ICatalogCollection* This,
        /* [annotation][in] */
        _In_  DISPID dispIdMember,
        /* [annotation][in] */
        _In_  REFIID riid,
        /* [annotation][in] */
        _In_  LCID lcid,
        /* [annotation][in] */
        _In_  WORD wFlags,
        /* [annotation][out][in] */
        _In_  DISPPARAMS* pDispParams,
        /* [annotation][out] */
        _Out_opt_  VARIANT* pVarResult,
        /* [annotation][out] */
        _Out_opt_  EXCEPINFO* pExcepInfo,
        /* [annotation][out] */
        _Out_opt_  UINT* puArgErr);

    /* [id][restricted][propget] */ HRESULT(STDMETHODCALLTYPE* get__NewEnum)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__deref_out_opt IUnknown** ppEnumVariant);

    /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_Item)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ long lIndex,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch** ppCatalogObject);

    /* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE* get_Count)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out long* plObjectCount);

    /* [helpstring] */ HRESULT(STDMETHODCALLTYPE* Remove)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ long lIndex);

    /* [helpstring] */ HRESULT(STDMETHODCALLTYPE* Add)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch** ppCatalogObject);

    /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* Populate)(
        __RPC__in ICatalogCollection* This);

    /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* SaveChanges)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out long* pcChanges);

    /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetCollection)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ __RPC__in BSTR bstrCollName,
        /* [in] */ VARIANT varObjectKey,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch** ppCatalogCollection);

    /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_Name)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out VARIANT* pVarNamel);

    /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_AddEnabled)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL* pVarBool);

    /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_RemoveEnabled)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL* pVarBool);

    /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetUtilInterface)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch** ppIDispatch);

    /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_DataStoreMajorVersion)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out long* plMajorVersion);

    /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_DataStoreMinorVersion)(
        __RPC__in ICatalogCollection* This,
        /* [retval][out] */ __RPC__out long* plMinorVersionl);

    /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* PopulateByKey)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ __RPC__in SAFEARRAY* psaKeys);

    /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* PopulateByQuery)(
        __RPC__in ICatalogCollection* This,
        /* [in] */ __RPC__in BSTR bstrQueryString,
        /* [in] */ long lQueryType);

    END_INTERFACE
} ICatalogCollectionVtbl;

// Interface pointer to vtable
interface ICatalogCollection
{
    CONST_VTBL struct ICatalogCollectionVtbl* lpVtbl;
};

// Macros for calling interface methods through the vtable
#ifdef COBJMACROS

#define ICatalogCollection_QueryInterface(This,riid,ppvObject) \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) )

#define ICatalogCollection_AddRef(This) \
    ( (This)->lpVtbl -> AddRef(This) )

#define ICatalogCollection_Release(This) \
    ( (This)->lpVtbl -> Release(This) )

#define ICatalogCollection_GetTypeInfoCount(This,pctinfo) \
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) )

#define ICatalogCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo) \
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) )

#define ICatalogCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) \
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) )

#define ICatalogCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) \
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) )

#define ICatalogCollection_get__NewEnum(This,ppEnumVariant) \
    ( (This)->lpVtbl -> get__NewEnum(This,ppEnumVariant) )

#define ICatalogCollection_get_Item(This,lIndex,ppCatalogObject) \
    ( (This)->lpVtbl -> get_Item(This,lIndex,ppCatalogObject) )

#define ICatalogCollection_get_Count(This,plObjectCount) \
    ( (This)->lpVtbl -> get_Count(This,plObjectCount) )

#define ICatalogCollection_Remove(This,lIndex) \
    ( (This)->lpVtbl -> Remove(This,lIndex) )

#define ICatalogCollection_Add(This,ppCatalogObject) \
    ( (This)->lpVtbl -> Add(This,ppCatalogObject) )

#define ICatalogCollection_Populate(This) \
    ( (This)->lpVtbl -> Populate(This) )

#define ICatalogCollection_SaveChanges(This,pcChanges) \
    ( (This)->lpVtbl -> SaveChanges(This,pcChanges) )

#define ICatalogCollection_GetCollection(This,bstrCollName,varObjectKey,ppCatalogCollection) \
    ( (This)->lpVtbl -> GetCollection(This,bstrCollName,varObjectKey,ppCatalogCollection) )

#define ICatalogCollection_get_Name(This,pVarNamel) \
    ( (This)->lpVtbl -> get_Name(This,pVarNamel) )

#define ICatalogCollection_get_AddEnabled(This,pVarBool) \
    ( (This)->lpVtbl -> get_AddEnabled(This,pVarBool) )

#define ICatalogCollection_get_RemoveEnabled(This,pVarBool) \
    ( (This)->lpVtbl -> get_RemoveEnabled(This,pVarBool) )

#define ICatalogCollection_GetUtilInterface(This,ppIDispatch) \
    ( (This)->lpVtbl -> GetUtilInterface(This,ppIDispatch) )

#define ICatalogCollection_get_DataStoreMajorVersion(This,plMajorVersion) \
    ( (This)->lpVtbl -> get_DataStoreMajorVersion(This,plMajorVersion) )

#define ICatalogCollection_get_DataStoreMinorVersion(This,plMinorVersionl) \
    ( (This)->lpVtbl -> get_DataStoreMinorVersion(This,plMinorVersionl) )

#define ICatalogCollection_PopulateByKey(This,psaKeys) \
    ( (This)->lpVtbl -> PopulateByKey(This,psaKeys) )

#define ICatalogCollection_PopulateByQuery(This,bstrQueryString,lQueryType) \
    ( (This)->lpVtbl -> PopulateByQuery(This,bstrQueryString,lQueryType) )

#endif /* COBJMACROS */

#endif /* __ICatalogCollection_INTERFACE_DEFINED__ */
