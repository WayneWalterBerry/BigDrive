// <copyright file="ShItemIdUtil.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <wchar.h>

#include "ShItemIdUtil.h"

//<inheritdoc />
HRESULT ShItemIdUtil::Serialize(_In_ const SHITEMID* shitemid, _Out_ BSTR& bstPath)
{
    if (shitemid == nullptr || &bstPath == nullptr)
    {
        return E_INVALIDARG;
    }

    // abID length is cb - sizeof(USHORT)
    USHORT abIdLen = shitemid->cb > sizeof(USHORT) ? shitemid->cb - sizeof(USHORT) : 0;
    if (abIdLen == 0)
    {
        bstPath = ::SysAllocString(L"");
        return S_OK;
    }

    // Allocate BSTR with the required length (no null terminator needed, BSTR is length-prefixed)
    bstPath = ::SysAllocStringLen(nullptr, static_cast<UINT>(abIdLen));
    if (bstPath == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    const BYTE* abID = reinterpret_cast<const BYTE*>(shitemid) + sizeof(USHORT);
    for (USHORT i = 0; i < abIdLen; ++i)
    {
        // Write each byte directly as a wchar_t
        bstPath[i] = static_cast<wchar_t>(abID[i]);
    }

    return S_OK;
}

//<inheritdoc />
HRESULT ShItemIdUtil::Deserialize(_In_ BSTR bstPath, _Out_ SHITEMID** pShitemid)
{
    if (bstPath == nullptr || pShitemid == nullptr)
    {
        return E_INVALIDARG;
    }

    *pShitemid = nullptr;
    UINT charLen = ::SysStringLen(bstPath);

    USHORT abIdLen = static_cast<USHORT>(charLen);
    USHORT cb = abIdLen + sizeof(USHORT);

    // Allocate memory for SHITEMID
    SHITEMID* shitemid = (SHITEMID*)::CoTaskMemAlloc(cb);
    if (!shitemid)
    {
        return E_OUTOFMEMORY;
    }
    shitemid->cb = cb;

    BYTE* abID = reinterpret_cast<BYTE*>(shitemid) + sizeof(USHORT);

    for (USHORT i = 0; i < abIdLen; ++i)
    {
        // Read each wchar_t as a byte
        abID[i] = static_cast<BYTE>(bstPath[i]);
    }

    *pShitemid = shitemid;
    return S_OK;
}