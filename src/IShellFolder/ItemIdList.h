// <copyright file="ItemIdList.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shtypes.h>

// Local
#include "ShellItemId.h"

class __declspec(dllexport) ItemIdList
{
private:

    LPITEMIDLIST m_pidl;

public:

    ItemIdList(LPITEMIDLIST pidl)
        : m_pidl(pidl)
    {
    }

    /// <summary>
    /// Retrieves the next ITEMID in the list.
    /// </summary>
    /// <param name="next">[out] Receives the next ItemIdList if available; otherwise, an empty ItemIdList.</param>
    /// <returns>S_OK if the next item exists, S_FALSE if at the end of the list, or E_POINTER if m_pidl is null.</returns>
    HRESULT NextItem(ItemIdList& next) const;

    /// <summary>
    /// Retrieves the first SHITEMID in this ItemIdList as a ShellItemId.
    /// </summary>
    /// <param name="itemId">[out] Receives the ShellItemId if available; otherwise, a default/null ShellItemId.</param>
    /// <returns>S_OK if a valid item exists, S_FALSE if the list is empty, or E_POINTER if m_pidl is null.</returns>
    HRESULT GetShellItemId(ShellItemId& itemId) const;

    /// <summary>
    /// Computes a hash value for the entire ItemIdList by recursively hashing each SHITEMID.
    /// </summary>
    /// <param name="seed">The initial hash value to combine with each item.</param>
    /// <returns>A uint32_t hash representing the entire ItemIdList.</returns>
    ULONG Hash(ULONG seed = 2166136261u) const;
};