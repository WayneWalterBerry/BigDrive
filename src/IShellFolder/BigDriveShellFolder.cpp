// <copyright file="BigDriveShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"

HRESULT BigDriveShellFolder::GetProviderCLSID(CLSID& clsidProvider) const
{
	return S_OK;
}

HRESULT BigDriveShellFolder::GetPath(BSTR& bstrPath)
{
	bstrPath = ::SysAllocString(L"\\");
	return S_OK;
}

// Allocates a BIGDRIVE_ITEMID as a valid SHITEMID for use in a PIDL.
// The returned pointer must be freed with CoTaskMemFree.
HRESULT BigDriveShellFolder::AllocateBigDriveItemId(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST& pidl)
{
    pidl = nullptr;

    if (!bstrName)
    {
        return E_INVALIDARG;
    }

    // Calculate the size needed for the name (including null terminator)
    size_t nameLen = (SysStringLen(bstrName) + 1) * sizeof(WCHAR);

    // The SHITEMID structure: [cb][uType][bstrName][terminator]
    size_t cb = sizeof(USHORT) + sizeof(INT) + nameLen;
    BYTE* buffer = (BYTE*)CoTaskMemAlloc(cb + sizeof(USHORT)); // +2 for PIDL terminator

    if (!buffer)
        return E_OUTOFMEMORY;

    // Set cb (size of this SHITEMID, including cb itself)
    *(USHORT*)buffer = static_cast<USHORT>(cb);

    // Set uType
    *(UINT*)(buffer + sizeof(USHORT)) = nType;

    // Copy bstrName after uType
    memcpy(buffer + sizeof(USHORT) + sizeof(UINT), bstrName, nameLen);

    // Add PIDL terminator (USHORT 0) after the SHITEMID
    *(USHORT*)(buffer + cb) = 0;

    pidl = reinterpret_cast<LPITEMIDLIST>(buffer);
    return S_OK;
}