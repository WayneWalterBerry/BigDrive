// <copyright file="BigDriveEnumIDListImports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\BigDriveEnumIDList.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) BigDriveEnumIDList* CreateBigDriveEnumIDList();
    __declspec(dllimport) BigDriveEnumIDList* CreateBigDriveEnumIDListWithCapacity(ULONG initialCapacity);
    __declspec(dllimport) BigDriveEnumIDList* CreateBigDriveEnumIDListWithItems(LPITEMIDLIST* pidls, ULONG count);
    __declspec(dllimport) void DestroyBigDriveEnumIDList(BigDriveEnumIDList* pEnum);

    __declspec(dllimport) HRESULT BigDriveEnumIDList_QueryInterface(BigDriveEnumIDList* pEnum, REFIID riid, void** ppv);
    __declspec(dllimport) ULONG BigDriveEnumIDList_AddRef(BigDriveEnumIDList* pEnum);
    __declspec(dllimport) ULONG BigDriveEnumIDList_Release(BigDriveEnumIDList* pEnum);

    __declspec(dllimport) HRESULT BigDriveEnumIDList_Next(BigDriveEnumIDList* pEnum, ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched);
    __declspec(dllimport) HRESULT BigDriveEnumIDList_Skip(BigDriveEnumIDList* pEnum, ULONG celt);
    __declspec(dllimport) HRESULT BigDriveEnumIDList_Reset(BigDriveEnumIDList* pEnum);
    __declspec(dllimport) HRESULT BigDriveEnumIDList_Clone(BigDriveEnumIDList* pEnum, IEnumIDList** ppenum);

    __declspec(dllimport) HRESULT BigDriveEnumIDList_Add(BigDriveEnumIDList* pEnum, LPITEMIDLIST pidl);

#ifdef __cplusplus
}
#endif