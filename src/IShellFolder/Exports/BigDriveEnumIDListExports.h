// <copyright file="BigDriveEnumIDListExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\BigDriveEnumIDList.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) BigDriveEnumIDList* CreateBigDriveEnumIDList();
    __declspec(dllexport) BigDriveEnumIDList* CreateBigDriveEnumIDListWithCapacity(ULONG initialCapacity); 
    __declspec(dllexport) BigDriveEnumIDList* CreateBigDriveEnumIDListWithItems(LPITEMIDLIST* pidls, ULONG count);
    __declspec(dllexport) void DestroyBigDriveEnumIDList(BigDriveEnumIDList* pEnum);

    __declspec(dllexport) HRESULT BigDriveEnumIDList_QueryInterface(BigDriveEnumIDList* pEnum, REFIID riid, void** ppv);
    __declspec(dllexport) ULONG BigDriveEnumIDList_AddRef(BigDriveEnumIDList* pEnum);
    __declspec(dllexport) ULONG BigDriveEnumIDList_Release(BigDriveEnumIDList* pEnum);

    __declspec(dllexport) HRESULT BigDriveEnumIDList_Next(BigDriveEnumIDList* pEnum, ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched);
    __declspec(dllexport) HRESULT BigDriveEnumIDList_Skip(BigDriveEnumIDList* pEnum, ULONG celt);
    __declspec(dllexport) HRESULT BigDriveEnumIDList_Reset(BigDriveEnumIDList* pEnum);
    __declspec(dllexport) HRESULT BigDriveEnumIDList_Clone(BigDriveEnumIDList* pEnum, IEnumIDList** ppenum);

    __declspec(dllexport) HRESULT BigDriveEnumIDList_Add(BigDriveEnumIDList* pEnum, LPITEMIDLIST pidl);

#ifdef __cplusplus
}
#endif