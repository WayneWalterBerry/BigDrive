// <copyright file="BigDriveShellFolderExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\BigDriveShellFolder.h"

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) HRESULT AllocateBigDriveItemIdExport(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST* ppidl);

#ifdef __cplusplus
}
#endif