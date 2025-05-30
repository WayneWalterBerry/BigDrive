// <copyright file="BigDriveShellFolderExports.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveShellFolderExports.h"

extern "C" {

    HRESULT AllocateBigDriveItemIdExport(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST* ppidl)
    {
        if (ppidl == nullptr)
        {
            return E_POINTER;
        }

        // Forward directly to the implementation in BigDriveShellFolder.cpp
        // This assumes the function is implemented as a free function, not a member.
        return BigDriveShellFolder::AllocateBigDriveItemId(nType, bstrName, *ppidl);
    }

    HRESULT GetBigDriveItemNameFromPidlExport(PCUITEMID_CHILD pidl, STRRET* pName)
    {
        return BigDriveShellFolder::GetBigDriveItemNameFromPidl(pidl, pName);
    }
}