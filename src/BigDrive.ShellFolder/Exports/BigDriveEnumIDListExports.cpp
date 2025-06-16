// <copyright file="BigDriveEnumIDListExports.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveEnumIDListExports.h"

extern "C" {

    BigDriveEnumIDList* CreateBigDriveEnumIDList()
    {
        return new BigDriveEnumIDList();
    }

    BigDriveEnumIDList* CreateBigDriveEnumIDListWithCapacity(ULONG initialCapacity)
    {
        return new BigDriveEnumIDList(initialCapacity);
    }

    BigDriveEnumIDList* CreateBigDriveEnumIDListWithItems(LPITEMIDLIST* pidls, ULONG count)
    {
        return new BigDriveEnumIDList(pidls, count);
    }

    void DestroyBigDriveEnumIDList(BigDriveEnumIDList* pEnum)
    {
        delete pEnum;
    }

    HRESULT BigDriveEnumIDList_QueryInterface(BigDriveEnumIDList* pEnum, REFIID riid, void** ppv)
    {
        if (!pEnum || !ppv) return E_POINTER;
        return pEnum->QueryInterface(riid, ppv);
    }

    ULONG BigDriveEnumIDList_AddRef(BigDriveEnumIDList* pEnum)
    {
        if (!pEnum) return 0;
        return pEnum->AddRef();
    }

    ULONG BigDriveEnumIDList_Release(BigDriveEnumIDList* pEnum)
    {
        if (!pEnum) return 0;
        return pEnum->Release();
    }

    HRESULT BigDriveEnumIDList_Next(BigDriveEnumIDList* pEnum, ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched)
    {
        if (!pEnum) return E_POINTER;
        return pEnum->Next(celt, rgelt, pceltFetched);
    }

    HRESULT BigDriveEnumIDList_Skip(BigDriveEnumIDList* pEnum, ULONG celt)
    {
        if (!pEnum) return E_POINTER;
        return pEnum->Skip(celt);
    }

    HRESULT BigDriveEnumIDList_Reset(BigDriveEnumIDList* pEnum)
    {
        if (!pEnum) return E_POINTER;
        return pEnum->Reset();
    }

    HRESULT BigDriveEnumIDList_Clone(BigDriveEnumIDList* pEnum, IEnumIDList** ppenum)
    {
        if (!pEnum || !ppenum) return E_POINTER;
        return pEnum->Clone(ppenum);
    }

    HRESULT BigDriveEnumIDList_Add(BigDriveEnumIDList* pEnum, LPITEMIDLIST pidl)
    {
        if (!pEnum) return E_POINTER;
        return pEnum->Add(pidl);
    }

} // extern "C"