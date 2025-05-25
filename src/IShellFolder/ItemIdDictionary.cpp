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

/// <inheritdoc />
HRESULT ItemIdDictionary::Serialize(_In_ LPCITEMIDLIST pidl, _Out_ BSTR& bstrPath)
{
    // Return empty string if pidl is null
    if (pidl == nullptr)
    {
        bstrPath = ::SysAllocString(L"");
        return S_OK;
    }

    if (::ILIsEqual(s_staticInitializer.GetDrivesPidl(), pidl))
    {
        bstrPath = ::SysAllocString(L"My PC");
        return S_OK;
    }

    // First, calculate the total length needed for the BSTR (in wchar_t)
    size_t totalCharLen = 0;
    size_t itemCount = 0;
    const BYTE* pCur = reinterpret_cast<const BYTE*>(pidl);

    // Walk the list to calculate length
    while (true)
    {
        const SHITEMID* shitemid = reinterpret_cast<const SHITEMID*>(pCur);
        if (shitemid->cb == 0)
            break;
        USHORT abIdLen = shitemid->cb > sizeof(USHORT) ? shitemid->cb - sizeof(USHORT) : 0;
        totalCharLen += abIdLen;
        ++itemCount;
        pCur += shitemid->cb;
    }

    if (itemCount == 0)
    {
        bstrPath = ::SysAllocString(L"");
        return S_OK;
    }

    // Add space for '/' separators (itemCount - 1)
    totalCharLen += (itemCount - 1);

    // Allocate BSTR
    bstrPath = ::SysAllocStringLen(nullptr, static_cast<UINT>(totalCharLen));
    if (bstrPath == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // Walk again and serialize
    pCur = reinterpret_cast<const BYTE*>(pidl);
    UINT pos = 0;
    size_t itemIdx = 0;
    while (true)
    {
        const SHITEMID* shitemid = reinterpret_cast<const SHITEMID*>(pCur);
        if (shitemid->cb == 0)
        {
            break;
        }

        USHORT abIdLen = shitemid->cb > sizeof(USHORT) ? shitemid->cb - sizeof(USHORT) : 0;
        const BYTE* abID = reinterpret_cast<const BYTE*>(shitemid) + sizeof(USHORT);

        for (USHORT i = 0; i < abIdLen; ++i)
        {
            // Directly cast each byte to wchar_t and store in the BSTR
            bstrPath[pos++] = static_cast<wchar_t>(abID[i]);
        }

        ++itemIdx;
        if (itemIdx < itemCount)
        {
            bstrPath[pos++] = L'/';
        }

        pCur += shitemid->cb;
    }

    // No need to null-terminate, SysAllocStringLen handles the length
    return S_OK;
}

/// <inheritdoc />
HRESULT ItemIdDictionary::Deserialize(_In_ const BSTR bstrPath, _Out_ LPITEMIDLIST* ppidl)
{
    if (!ppidl)
    {
        return E_POINTER;
    }

    *ppidl = nullptr;

    // Handle empty or null input string: allocate a single terminator
    if (!bstrPath || bstrPath[0] == L'\0')
    {
        *ppidl = (LPITEMIDLIST)::CoTaskMemAlloc(sizeof(USHORT));
        if (!*ppidl)
        {
            return E_OUTOFMEMORY;
        }
        *(USHORT*)*ppidl = 0;
        return S_OK;
    }

    // First pass: count items and their abID lengths
    const wchar_t* p = bstrPath;
    int itemCount = 1;
    int curLen = 0;
    int abidLensStack[16];
    int* abidLens = abidLensStack;
    int abidLensCapacity = 16;

    // Count items and their lengths
    for (const wchar_t* q = p; *q; ++q)
    {
        if (*q == L'/')
        {
            ++itemCount;
        }
    }

    if (itemCount > abidLensCapacity)
    {
        abidLens = (int*)::CoTaskMemAlloc(itemCount * sizeof(int));
        if (!abidLens)
        {
            return E_OUTOFMEMORY;
        }
    }

    // Second pass: fill abidLens
    int i = 0;
    curLen = 0;
    for (const wchar_t* q = p; ; ++q)
    {
        if (*q == L'/' || *q == L'\0')
        {
            abidLens[i++] = curLen;
            curLen = 0;
            if (*q == L'\0')
            {
                break;
            }
        }
        else
        {
            ++curLen;
        }
    }

    // Calculate total size for ITEMIDLIST: each SHITEMID = sizeof(USHORT) + abidLen, plus terminator
    size_t total = 0;
    for (i = 0; i < itemCount; ++i)
    {
        total += sizeof(USHORT) + abidLens[i];
    }
    total += sizeof(USHORT); // terminator

    // Allocate buffer for ITEMIDLIST
    BYTE* buffer = (BYTE*)::CoTaskMemAlloc(total);
    if (!buffer)
    {
        if (abidLens != abidLensStack)
        {
            ::CoTaskMemFree(abidLens);
        }
        return E_OUTOFMEMORY;
    }

    // Parse BSTR directly into the buffer
    BYTE* cur = buffer;
    p = bstrPath;
    for (i = 0; i < itemCount; ++i)
    {
        USHORT cb = static_cast<USHORT>(sizeof(USHORT) + abidLens[i]);
        *(USHORT*)cur = cb;
        BYTE* abID = cur + sizeof(USHORT);
        for (int j = 0; j < abidLens[i]; ++j)
        {
            abID[j] = static_cast<BYTE>(p[j]);
        }
        p += abidLens[i];
        if (*p == L'/')
        {
            ++p;
        }
        cur += cb;
    }

    // Add terminator
    *(USHORT*)cur = 0;
    *ppidl = reinterpret_cast<LPITEMIDLIST>(buffer);

    if (abidLens != abidLensStack)
    {
        ::CoTaskMemFree(abidLens);
    }

    return S_OK;
}