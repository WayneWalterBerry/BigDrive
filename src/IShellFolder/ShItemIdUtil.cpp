// <copyright file="ShItemIdUtil.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <wchar.h>

#include "ShItemIdUtil.h"

/// <inheritdoc />
HRESULT ShItemIdUtil::Serialize(_In_ const SHITEMID* shitemid, _Out_ BSTR& bstPath)
{
    if (shitemid == nullptr || bstPath == nullptr)
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

    // Each byte becomes two hex chars
    size_t hexLen = abIdLen * 2;

    // Allocate BSTR with the required length (no null terminator needed, BSTR is length-prefixed)
    bstPath = ::SysAllocStringLen(nullptr, static_cast<UINT>(hexLen));
    if (bstPath == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    const BYTE* abID = reinterpret_cast<const BYTE*>(shitemid) + sizeof(USHORT);
    for (USHORT i = 0; i < abIdLen; ++i)
    {
        // Write two hex digits per byte directly into the BSTR buffer
        unsigned char byte = abID[i];
        bstPath[i * 2] = (wchar_t)L"0123456789ABCDEF"[byte >> 4];
        bstPath[i * 2 + 1] = (wchar_t)L"0123456789ABCDEF"[byte & 0x0F];
    }

    // No need to null-terminate, SysAllocStringLen handles the length
    return S_OK;
}

/// <inheritdoc />
HRESULT ShItemIdUtil::Deserialize(_In_ BSTR bstPath, _Out_ SHITEMID** pShitemid)
{
    if (bstPath == nullptr || pShitemid == nullptr)
    {
        return E_INVALIDARG;
    }

    *pShitemid = nullptr;
    UINT hexLen = ::SysStringLen(bstPath);

    // Must be even number of hex digits
    if (hexLen % 2 != 0)
    {
        return E_INVALIDARG;
    }

    USHORT abIdLen = static_cast<USHORT>(hexLen / 2);
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
        wchar_t high = bstPath[i * 2];
        wchar_t low = bstPath[i * 2 + 1];

        auto hex2nibble = [](wchar_t c) -> int {
            if (c >= L'0' && c <= L'9') return c - L'0';
            if (c >= L'A' && c <= L'F') return c - L'A' + 10;
            if (c >= L'a' && c <= L'f') return c - L'a' + 10;
            return -1;
            };

        int hi = hex2nibble(high);
        int lo = hex2nibble(low);
        if (hi < 0 || lo < 0)
        {
            ::CoTaskMemFree(shitemid);
            return E_INVALIDARG;
        }
        abID[i] = static_cast<BYTE>((hi << 4) | lo);
    }

    *pShitemid = shitemid;
    return S_OK;
}