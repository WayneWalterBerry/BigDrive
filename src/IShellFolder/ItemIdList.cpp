// <copyright file="ItemIdList.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "ItemIdList.h"

/// <summary>
/// Retrieves the first SHITEMID in this ItemIdList as a ShellItemId.
/// </summary>
/// <param name="itemId">[out] Receives the ShellItemId if available; otherwise, a default/null ShellItemId.</param>
/// <returns>S_OK if a valid item exists, S_FALSE if the list is empty, or E_POINTER if m_pidl is null.</returns>
HRESULT ItemIdList::GetShellItemId(ShellItemId& itemId) const
{
    const SHITEMID shitemid = m_pidl->mkid;
    if (shitemid.cb == 0)
    {
        return S_FALSE;
    }

    itemId = ShellItemId(&shitemid);
    return S_OK;
}

/// <summary>
/// Retrieves the next ITEMID in the list.
/// </summary>
/// <param name="next">[out] Receives the next ItemIdList if available; otherwise, an empty ItemIdList.</param>
/// <returns>S_OK if the next item exists, S_FALSE if at the end of the list, or E_POINTER if m_pidl is null.</returns>
HRESULT ItemIdList:: NextItem(ItemIdList& next) const
{
    next = ItemIdList(nullptr);

    const BYTE* current = reinterpret_cast<const BYTE*>(&m_pidl->mkid);
    USHORT cb = reinterpret_cast<const SHITEMID*>(current)->cb;

    if (cb == 0)
    {
        // End of list
        return S_FALSE; 
    }

    const BYTE* nextPtr = current + cb;
    const SHITEMID* nextItem = reinterpret_cast<const SHITEMID*>(nextPtr);

    if (nextItem->cb == 0)
    {
        // No more items
        return S_FALSE;
    }

    next = ItemIdList(reinterpret_cast<LPITEMIDLIST>(const_cast<BYTE*>(nextPtr)));
    return S_OK;
}

/// <summary>
/// Computes a hash value for the entire ItemIdList by recursively hashing each SHITEMID.
/// </summary>
/// <param name="seed">The initial hash value to combine with each item.</param>
/// <returns>A size_t hash representing the entire ItemIdList.</returns>
ULONG ItemIdList::Hash(ULONG seed) const
{
    if (!m_pidl || m_pidl->mkid.cb == 0)
    {
        return seed;
    }

    ULONG hash = seed;
    const BYTE* ptr = reinterpret_cast<const BYTE*>(&m_pidl->mkid);

    while (true)
    {
        const SHITEMID* shitemid = reinterpret_cast<const SHITEMID*>(ptr);
        if (shitemid->cb == 0)
        {
            break;
        }

        ShellItemId shellItemId(shitemid);
        hash ^= shellItemId.Hash();
        hash *= 16777619u; // FNV-1a 32-bit prime

        ptr += shitemid->cb;
    }

    return hash;
}

