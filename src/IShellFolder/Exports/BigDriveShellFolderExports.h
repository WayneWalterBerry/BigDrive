// <copyright file="BigDriveShellFolderExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\BigDriveShellFolder.h"

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) HRESULT AllocateBigDriveItemIdExport(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST* ppidl);

    /// <summary>
    /// Extracts the Unicode name (as a BSTR) from the last BIGDRIVE_ITEMID in the given PIDL chain.
    /// The caller is responsible for freeing the BSTR using SysFreeString.
    /// </summary>
    __declspec(dllexport) HRESULT GetBigDriveItemNameFromPidlExport(PCUITEMID_CHILD pidl, STRRET* pName);


#ifdef __cplusplus
}
#endif