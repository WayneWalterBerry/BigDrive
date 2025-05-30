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
    : m_refCount(1), m_index(0), m_count(count), m_capacity(count), m_pidls(nullptr)
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
/// Constructs a new BigDriveEnumIDList with a preallocated buffer for the specified number of PIDLs.
/// </summary>
BigDriveEnumIDList::BigDriveEnumIDList(ULONG initialCapacity)
    : m_refCount(1), m_index(0), m_count(0), m_capacity(initialCapacity), m_pidls(nullptr)
{
    if (initialCapacity > 0)
    {
        m_pidls = new LPITEMIDLIST[initialCapacity];
        for (ULONG i = 0; i < initialCapacity; ++i)
        {
            m_pidls[i] = nullptr;
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
    {
        return E_INVALIDARG;
    }

    if (m_count < m_capacity)
    {
        m_pidls[m_count] = ::ILClone(pidl);
        if (!m_pidls[m_count])
        {
            return E_OUTOFMEMORY;
        }

		// Successfully added the PIDL, increment the count
        ++m_count;
        return S_OK;
    }

    // Need to grow the buffer
    ULONG newCapacity = (m_capacity == 0) ? 4 : m_capacity * 2;

    LPITEMIDLIST* newArray = new LPITEMIDLIST[newCapacity];
    if (!newArray)
    {
        return E_OUTOFMEMORY;
    }

    for (ULONG i = 0; i < m_count; ++i)
    {
        newArray[i] = m_pidls[i];
    }

    newArray[m_count] = ::ILClone(pidl);
    if (!newArray[m_count])
    {
        delete[] newArray;
        return E_OUTOFMEMORY;
    }

    for (ULONG i = m_count + 1; i < newCapacity; ++i)
    {
        newArray[i] = nullptr;
    }

    if (m_pidls)
    {
        delete[] m_pidls;
		m_pidls = nullptr;
    }

    m_pidls = newArray;
    ++m_count;
    m_capacity = newCapacity;

    return S_OK;
}