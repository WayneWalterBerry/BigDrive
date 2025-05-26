// <copyright file="ItemIdDictionary.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>
// <summary>
//   Implementation of a minimal dictionary mapping ITEMID (PIDL) keys to LPWSTR values.
//   This implementation does not use the C++ Standard Library and is suitable for
//   use in shell extensions. The dictionary uses a simple hash table with separate chaining.
//   All methods return HRESULTs and use output parameters for results.
// </summary>

#include "pch.h"
#include "ItemIdDictionary.h"
#include "ItemIdDictionaryStaticInitializer.h"

#include <shlwapi.h>

// Static member definition
ItemIdDictionaryStaticInitializer ItemIdDictionary::s_staticInitializer;

ItemIdDictionary::ItemIdDictionary()
{
    for (int i = 0; i < TABLE_SIZE; ++i)
    {
        m_buckets[i] = nullptr;
    }
}

ItemIdDictionary::~ItemIdDictionary()
{
    Clear();
}

/// <inheritdoc />
int ItemIdDictionary::HashPidl(LPCITEMIDLIST pidl) const
{
    int hash = 0;
    const BYTE* p = (const BYTE*)pidl;
    if (!pidl)
    {
        return 0;
    }

    while (true)
    {
        USHORT cb = *(USHORT*)p;
        if (cb == 0)
            break;
        for (USHORT i = 0; i < cb; ++i)
            hash += p[i];
        p += cb;
    }
    return (hash < 0 ? -hash : hash) % TABLE_SIZE;
}

/// <inheritdoc />
BOOL ItemIdDictionary::ComparePidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) const
{
    if (!pidl1 || !pidl2)
    {
        return FALSE;
    }

    return ::ILIsEqual(pidl1, pidl2);
}

/// <inheritdoc />
HRESULT ItemIdDictionary::Insert(LPCITEMIDLIST key, LPCWSTR value)
{
    HRESULT hr = S_OK;
    int idx = 0;
    ItemIdDictionaryEntry* entry = nullptr;
    ItemIdDictionaryEntry* newEntry = nullptr;
    size_t len = 0;

    // Validate input parameters
    if (!key || !value)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    // Compute the hash bucket index for the given PIDL key
    idx = HashPidl(key);
    entry = m_buckets[idx];

    // Search for an existing entry with the same PIDL key
    while (entry)
    {
        if (ComparePidls(entry->key, key))
        {
            // If found, update the value (free old value, allocate and copy new one)
            if (entry->value)
            {
                ::CoTaskMemFree(entry->value);
            }

            len = (wcslen(value) + 1) * sizeof(WCHAR);
            entry->value = (LPWSTR)::CoTaskMemAlloc(len);
            if (!entry->value)
            {
                hr = E_OUTOFMEMORY;
                goto End;
            }

            ::wcscpy_s(entry->value, len / sizeof(WCHAR), value);
            goto End;
        }

        entry = entry->next;
    }

    // No existing entry found, create a new dictionary entry
    newEntry = (ItemIdDictionaryEntry*)::CoTaskMemAlloc(sizeof(ItemIdDictionaryEntry));
    if (!newEntry)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Clone the PIDL key for storage in the dictionary
    newEntry->key = ::ILClone(key);
    if (!newEntry->key)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Allocate and copy the value string
    len = (wcslen(value) + 1) * sizeof(WCHAR);
    newEntry->value = (LPWSTR)::CoTaskMemAlloc(len);

    if (!newEntry->value)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    wcscpy_s(newEntry->value, len / sizeof(WCHAR), value);

    // Insert the new entry at the head of the bucket's linked list
    newEntry->next = m_buckets[idx];
    m_buckets[idx] = newEntry;

End:

    // Clean up in case of failure (free any partially allocated resources)
    if (FAILED(hr) && newEntry)
    {
        if (newEntry->key)
        {
            ::ILFree(newEntry->key);
        }

        if (newEntry->value)
        {
            ::CoTaskMemFree(newEntry->value);
        }

        ::CoTaskMemFree(newEntry);
    }

    return hr;
}

/// <inheritdoc />
HRESULT ItemIdDictionary::Lookup(LPCITEMIDLIST key, LPCWSTR* outValue) const
{
    HRESULT hr = S_OK;
    int idx = 0;
    ItemIdDictionaryEntry* entry = nullptr;

    if (!outValue)
    {
        hr = E_POINTER;
        goto End;
    }
    *outValue = nullptr;

    if (!key)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    idx = HashPidl(key);
    entry = m_buckets[idx];
    while (entry)
    {
        if (ComparePidls(entry->key, key))
        {
            *outValue = entry->value;
            hr = S_OK;
            goto End;
        }
        entry = entry->next;
    }
    hr = S_FALSE;

End:

    return hr;
}

/// <inheritdoc />
HRESULT ItemIdDictionary::Remove(LPCITEMIDLIST key)
{
    HRESULT hr = S_OK;
    int idx = 0;
    ItemIdDictionaryEntry* prev = nullptr;
    ItemIdDictionaryEntry* entry = nullptr;

    if (!key)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    idx = HashPidl(key);
    entry = m_buckets[idx];
    while (entry)
    {
        if (ComparePidls(entry->key, key))
        {
            if (prev)
                prev->next = entry->next;
            else
                m_buckets[idx] = entry->next;
            if (entry->key)
                ::ILFree(entry->key);
            if (entry->value)
                ::CoTaskMemFree(entry->value);
            ::CoTaskMemFree(entry);
            hr = S_OK;
            goto End;
        }
        prev = entry;
        entry = entry->next;
    }
    hr = S_FALSE;

End:

    return hr;
}

/// <inheritdoc />
HRESULT ItemIdDictionary::Clear()
{
    HRESULT hr = S_OK;
    int i = 0;
    ItemIdDictionaryEntry* entry = nullptr;
    ItemIdDictionaryEntry* next = nullptr;

    for (i = 0; i < TABLE_SIZE; ++i)
    {
        entry = m_buckets[i];
        while (entry)
        {
            next = entry->next;
            if (entry->key)
                ::ILFree(entry->key);
            if (entry->value)
                ::CoTaskMemFree(entry->value);
            ::CoTaskMemFree(entry);
            entry = next;
        }
        m_buckets[i] = nullptr;
    }

    return hr;
}
