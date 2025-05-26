// <copyright file="BigDriveEnumIDList.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
//   Implements the BigDriveEnumIDList class, a custom IEnumIDList for enumerating PIDLs
//   in the BigDrive shell extension.
// </summary>

#include "pch.h"

#include "BigDriveEnumIDList.h"
#include <shlobj.h>

/// <summary>
/// Constructs a new BigDriveEnumIDList for the given array of PIDLs.
/// The enumerator clones each PIDL and manages their lifetime.
/// </summary>
BigDriveEnumIDList::BigDriveEnumIDList(LPITEMIDLIST* pidls, ULONG count)
    : m_refCount(1), m_index(0), m_count(count), m_pidls(nullptr)
{
    if (count > 0 && pidls) 
    {
        m_pidls = new LPITEMIDLIST[count];
        for (ULONG i = 0; i < count; ++i) 
        {
            m_pidls[i] = ILClone(pidls[i]);
        }
    }
}

/// <summary>
/// Destructor. Releases all cloned PIDLs and internal resources.
/// </summary>
BigDriveEnumIDList::~BigDriveEnumIDList()
{
    if (m_pidls) 
    {
        for (ULONG i = 0; i < m_count; ++i) 
        {
            if (m_pidls[i]) ILFree(m_pidls[i]);
        }

        delete[] m_pidls;
    }
}

/// <summary>
/// Adds a new PIDL to the enumerator. The PIDL is cloned and managed by the enumerator.
/// </summary>
/// <param name="pidl">The PIDL to add. This method clones the PIDL.</param>
/// <returns>S_OK if added successfully; E_OUTOFMEMORY if allocation fails; E_INVALIDARG if pidl is nullptr.</returns>
HRESULT BigDriveEnumIDList::Add(LPITEMIDLIST pidl)
{
    if (!pidl)
        return E_INVALIDARG;

    // Allocate new array
    LPITEMIDLIST* newArray = new LPITEMIDLIST[m_count + 1];
    if (!newArray)
    {
        return E_OUTOFMEMORY;
    }

    // Copy existing pointers
    for (ULONG i = 0; i < m_count; ++i)
    {
        newArray[i] = m_pidls[i];
    }

    // Clone and add the new PIDL
    newArray[m_count] = ILClone(pidl);
    if (!newArray[m_count])
    {
        delete[] newArray;
        return E_OUTOFMEMORY;
    }

    // Delete old array (but not the PIDLs themselves, as they are moved)
    if (m_pidls)
        delete[] m_pidls;

    m_pidls = newArray;
    ++m_count;
    return S_OK;
}