// <copyright file="BigDriveEnumIDList.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
//   Declares the BigDriveEnumIDList class, a custom implementation of IEnumIDList
//   for enumerating an array of PIDLs in the BigDrive shell extension.
// </summary>

#pragma once

#include <shlobj.h>

/// <summary>
/// Implements IEnumIDList for enumerating a known array of PIDLs.
/// Each PIDL is cloned and managed by this enumerator.
/// </summary>
class BigDriveEnumIDList : public IEnumIDList
{
    LONG m_refCount;
    ULONG m_index;
    ULONG m_count;
    LPITEMIDLIST* m_pidls;

public:

    /// <summary>
    /// Default constructor. Initializes an empty enumerator with no PIDLs.
    /// </summary>
    BigDriveEnumIDList()
        : m_refCount(1), m_index(0), m_count(0), m_pidls(nullptr)
    {
    }

    /// <summary>
    /// Constructs a new BigDriveEnumIDList for the given array of PIDLs.
    /// The enumerator clones each PIDL and manages their lifetime.
    /// </summary>
    /// <param name="pidls">Array of LPITEMIDLIST to enumerate.</param>
    /// <param name="count">Number of PIDLs in the array.</param>
    BigDriveEnumIDList(LPITEMIDLIST* pidls, ULONG count);

    /// <summary>
    /// Destructor. Releases all cloned PIDLs and internal resources.
    /// </summary>
    virtual ~BigDriveEnumIDList();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IUnknown methods

    /// <summary>
    /// Queries for a supported interface (IUnknown or IEnumIDList).
    /// </summary>
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override;

    /// <summary>
    /// Increments the reference count.
    /// </summary>
    ULONG __stdcall AddRef() override;

    /// <summary>
    /// Decrements the reference count and deletes the object if it reaches zero.
    /// </summary>
    ULONG __stdcall Release() override;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IEnumIDList methods
    
    /// <summary>
    /// Retrieves the next set of PIDLs in the enumeration sequence.
    /// </summary>
    /// <param name="celt">Number of PIDLs to retrieve.</param>
    /// <param name="rgelt">Array to receive the PIDLs.</param>
    /// <param name="pceltFetched">Receives the number of PIDLs actually fetched.</param>
    /// <returns>S_OK if the requested number was fetched, S_FALSE if fewer were available.</returns>
    HRESULT __stdcall Next(ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched) override;

    /// <summary>
    /// Skips the specified number of PIDLs in the enumeration sequence.
    /// </summary>
    /// <param name="celt">Number of PIDLs to skip.</param>
    /// <returns>S_OK if skipped, S_FALSE if end was reached.</returns>
    HRESULT __stdcall Skip(ULONG celt) override;

    /// <summary>
    /// Resets the enumeration sequence to the beginning.
    /// </summary>
    HRESULT __stdcall Reset() override;

    /// <summary>
    /// Creates a new enumerator with the same state as the current one.
    /// </summary>
    /// <param name="ppenum">Receives the new enumerator instance.</param>
    HRESULT __stdcall Clone(IEnumIDList** ppenum) override;

public:

    HRESULT Add(LPITEMIDLIST pidl);
};