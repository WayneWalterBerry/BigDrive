// <copyright file="BigDriveExtension-IShellExtInit.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveExtension.h"

STDMETHODIMP BigDriveExtension::Initialize(LPCITEMIDLIST /*pidlFolder*/, IDataObject* /*pDataObj*/, HKEY /*hKeyProgID*/)
{
    // No initialization needed for this example.
    return S_OK;
}