// <copyright file="BigDriveShellFolderImports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\BigDriveShellFolder.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) HRESULT AllocBigDrivePidlExport(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST* ppidl);

    /// <summary>
    /// Extracts the Unicode name (as a BSTR) from the last BIGDRIVE_ITEMID in the given PIDL chain.
    /// The caller is responsible for freeing the BSTR using SysFreeString.
    /// </summary>
    __declspec(dllimport) HRESULT GetBigDriveItemNameFromPidlExport(PCUITEMID_CHILD pidl, STRRET* pName);

    __declspec(dllimport) HRESULT GetPathExport(LPCITEMIDLIST pidl, int nSkip, BSTR& bstrPath);

#ifdef __cplusplus
}
#endif