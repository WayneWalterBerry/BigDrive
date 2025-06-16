// <copyright file="BigDriveEnumIDList-IEnumIDList.cpp" company="Wayne Walter Berry">
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
/// Retrieves the next set of PIDLs in the enumeration sequence.
/// </summary>
HRESULT __stdcall BigDriveEnumIDList::Next(ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched)
{
    if (!rgelt) return E_POINTER;

    ULONG fetched = 0;

    while (fetched < celt && m_index < m_count)
    {
        rgelt[fetched] = ILClone(m_pidls[m_index]);
        ++m_index;
        ++fetched;
    }

    if (pceltFetched)
    {
        *pceltFetched = fetched;
    }

    return (fetched == celt) ? S_OK : S_FALSE;
}

/// <summary>
/// Skips the specified number of PIDLs in the enumeration sequence.
/// </summary>
HRESULT __stdcall BigDriveEnumIDList::Skip(ULONG celt)
{
    m_index += celt;
    if (m_index > m_count) m_index = m_count;
    return (m_index < m_count) ? S_OK : S_FALSE;
}

/// <summary>
/// Resets the enumeration sequence to the beginning.
/// </summary>
HRESULT __stdcall BigDriveEnumIDList::Reset()
{
    m_index = 0;
    return S_OK;
}

/// <summary>
/// Creates a new enumerator with the same state as the current one.
/// </summary>
HRESULT __stdcall BigDriveEnumIDList::Clone(IEnumIDList** ppenum)
{
    if (!ppenum) return E_POINTER;
    BigDriveEnumIDList* pClone = new BigDriveEnumIDList(m_pidls, m_count);
    pClone->m_index = m_index;
    *ppenum = pClone;
    return S_OK;
}