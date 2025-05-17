// <copyright file="ShellItemId.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shtypes.h>

class __declspec(dllexport) ShellItemId
{
private:

    /// <summary>
    /// Object identifiers in the explorer's name space (ItemID and IDList)
    ///
    ///  All the items that the user can browse with the explorer (such as files,
    /// directories, servers, work-groups, etc.) has an identifier which is unique
    /// among items within the parent folder. Those identifiers are called item
    /// IDs (SHITEMID). Since all its parent folders have their own item IDs,
    /// any items can be uniquely identified by a list of item IDs, which is called
    /// an ID list (ITEMIDLIST).
    ///
    ///  ID lists are almost always allocated by the task allocator (see some
    /// description below as well as OLE 2.0 SDK) and may be passed across
    /// some of shell interfaces (such as IShellFolder). Each item ID in an ID list
    /// is only meaningful to its parent folder (which has generated it), and all
    /// the clients must treat it as an opaque binary data except the first two
    /// bytes, which indicates the size of the item ID.
    ///
    ///  When a shell extension -- which implements the IShellFolder interface --
    /// generates an item ID, it may put any information in it, not only the data
    /// with that it needs to identify the item, but also some additional
    /// information, which would help implementing some other functions efficiently.
    /// For example, the shell's IShellFolder implementation of file system items
    /// stores the primary (long) name of a file or a directory as the item
    /// identifier, but it also stores its alternative (short) name, size and date
    /// etc.
    ///
    ///  When an ID list is passed to one of shell APIs (such as SHGetPathFromIDList),
    /// it is always an absolute path -- relative from the root of the name space,
    /// which is the desktop folder. When an ID list is passed to one of IShellFolder
    /// member function, it is always a relative path from the folder (unless it
    /// is explicitly specified).
    ///
    /// ===========================================================================
    ///
    /// SHITEMID -- Item ID  (mkid)
    ///     USHORT      cb;             // Size of the ID (including cb itself)
    ///     BYTE        abID[];         // The item ID (variable length)
    ///
    /// ===========================================================================
    /// </summary>
    const SHITEMID *m_pShellItemId;

public:

    ShellItemId(const SHITEMID *pShellItemId)
        : m_pShellItemId(pShellItemId)
    {
    }

    /// <summary>
    /// Computes a simple FNV-1a hash of the abID data in m_shItemId.
    /// </summary>
    /// <returns>A size_t hash of the abID contents.</returns>
    ULONG Hash() const
    {
        if (m_pShellItemId->cb <= sizeof(USHORT))
        {
            return 0;
        }

        ULONG hash = 2166136261u; // FNV-1a 32-bit offset basis
        USHORT abidLen = m_pShellItemId->cb - sizeof(USHORT);
        for (USHORT i = 0; i < abidLen; ++i)
        {
            hash ^= m_pShellItemId->abID[i];
            hash *= 16777619u; // FNV-1a 32-bit prime
        }

        return hash;
    }
};
