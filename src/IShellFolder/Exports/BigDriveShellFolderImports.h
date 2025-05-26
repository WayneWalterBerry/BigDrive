// <copyright file="BigDriveShellFolderImports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\BigDriveShellFolder.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) HRESULT AllocateBigDriveItemIdExport(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST* ppidl);

#ifdef __cplusplus
}
#endif