// <copyright file="ILExtensions.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

/// <summary>
/// Counts the number of SHITEMID elements in a PIDL (Pointer to an Item ID List).
/// </summary>
/// <param name="pidl">
///   [in] Pointer to the first ITEMIDLIST structure in the PIDL chain.
/// </param>
/// <returns>
///   The number of SHITEMID elements in the PIDL, not including the terminating zero-length SHITEMID.
///   Returns 0 if the input PIDL is null or empty.
/// </returns>
inline UINT ILGetCount(LPCITEMIDLIST pidl)
{
    UINT count = 0;

    if (!pidl)
    {
        return 0;
    }

    while (pidl->mkid.cb)
    {
        ++count;
        pidl = (LPCITEMIDLIST)(((BYTE*)pidl) + pidl->mkid.cb);
    }

    return count;
}

/// <summary>
/// Retrieves a pointer to the SHITEMID at the specified zero-based index in a PIDL chain.
/// </summary>
/// <param name="pidl">[in] Pointer to the first ITEMIDLIST structure in the PIDL chain.</param>
/// <param name="index">[in] Zero-based index of the SHITEMID to retrieve.</param>
/// <param name="ppItemId">[out] Receives a pointer to the SHITEMID at the specified index, or nullptr if not found.</param>
/// <returns>S_OK if found; E_INVALIDARG if pidl is null or index is out of bounds.</returns>
inline HRESULT ILGetItemAt(LPCITEMIDLIST pidl, UINT index, const SHITEMID** ppItemId)
{
    if (!pidl || !ppItemId)
    {
        return E_INVALIDARG;
    }

    *ppItemId = nullptr;
    UINT current = 0;
    const BYTE* p = reinterpret_cast<const BYTE*>(pidl);

    while (true)
    {
        const SHITEMID* pItem = reinterpret_cast<const SHITEMID*>(p);
        if (pItem->cb == 0)
        {
            break;
        }

        if (current == index)
        {
            *ppItemId = pItem;
            return S_OK;
        }

        p += pItem->cb;
        ++current;
    }

    return E_INVALIDARG;
}