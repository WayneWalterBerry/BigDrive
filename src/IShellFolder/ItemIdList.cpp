// <copyright file="ItemIdList.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "ItemIdList.h"

/// <inheritdoc />
HRESULT ItemIdList::NextItem(ItemIdList& next) const
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

/// <inheritdoc />
HRESULT ItemIdList::SerializeList(_Out_ BSTR& bstPath)
{
    if (&bstPath == nullptr)
    {
        return E_INVALIDARG;
    }

    // First, calculate the total length needed for the BSTR
    size_t totalHexLen = 0;
    size_t itemCount = 0;
    const BYTE* pCur = reinterpret_cast<const BYTE*>(m_pidl);

    // Walk the list to calculate length
    while (true)
    {
        const SHITEMID* shitemid = reinterpret_cast<const SHITEMID*>(pCur);
        if (shitemid->cb == 0)
            break;
        USHORT abIdLen = shitemid->cb > sizeof(USHORT) ? shitemid->cb - sizeof(USHORT) : 0;
        totalHexLen += abIdLen * 2;
        ++itemCount;
        pCur += shitemid->cb;
    }

    if (itemCount == 0)
    {
        bstPath = ::SysAllocString(L"");
        return S_OK;
    }

    // Add space for '/' separators (itemCount - 1)
    totalHexLen += (itemCount - 1);

    // Allocate BSTR
    bstPath = ::SysAllocStringLen(nullptr, static_cast<UINT>(totalHexLen));
    if (bstPath == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // Walk again and serialize
    pCur = reinterpret_cast<const BYTE*>(m_pidl);
    UINT pos = 0;
    size_t itemIdx = 0;
    while (true)
    {
        const SHITEMID* shitemid = reinterpret_cast<const SHITEMID*>(pCur);
        if (shitemid->cb == 0)
            break;
        USHORT abIdLen = shitemid->cb > sizeof(USHORT) ? shitemid->cb - sizeof(USHORT) : 0;
        const BYTE* abID = reinterpret_cast<const BYTE*>(shitemid) + sizeof(USHORT);
        for (USHORT i = 0; i < abIdLen; ++i)
        {
            unsigned char byte = abID[i];
            bstPath[pos++] = (wchar_t)L"0123456789ABCDEF"[byte >> 4];
            bstPath[pos++] = (wchar_t)L"0123456789ABCDEF"[byte & 0x0F];
        }
        ++itemIdx;
        if (itemIdx < itemCount)
        {
            bstPath[pos++] = L'/';
        }
        pCur += shitemid->cb;
    }

    // No need to null-terminate, SysAllocStringLen handles the length
    return S_OK;
}

/// <inheritdoc />
HRESULT ItemIdList::DeserializeList(_In_ BSTR bstrPath, _Out_ LPITEMIDLIST* ppidl)
{
    // Validate output pointer
    if (!ppidl)
    {
        return E_POINTER;
    }

    *ppidl = nullptr;

    // Handle empty or null input string: allocate a single terminator
    if (!bstrPath || bstrPath[0] == L'\0')
    {
        // Allocate space for just the terminator (USHORT 0)
        *ppidl = (LPITEMIDLIST)::CoTaskMemAlloc(sizeof(USHORT));
        if (!*ppidl)
        {
            return E_OUTOFMEMORY;
        }
        *(USHORT*)*ppidl = 0;
        return S_OK;
    }

    // First pass: count the number of items and total abID length
    const wchar_t* p = bstrPath;
    int itemCount = 1;      // At least one item (no separator yet)
    int totalAbidLen = 0;   // Total length of all abID data
    int i = 0;

    // Count the number of '/' separators to determine item count
    for (const wchar_t* q = p; *q; ++q)
    {
        if (*q == L'/')
        {
            ++itemCount;
        }
    }

    // Allocate array to hold the length of each abID
    int* abidLens = (int*)::CoTaskMemAlloc(itemCount * sizeof(int));
    if (!abidLens)
    {
        return E_OUTOFMEMORY;
    }
    for (i = 0; i < itemCount; ++i)
    {
        abidLens[i] = 0;
    }

    // Second pass: determine the length (in bytes) of each abID
    i = 0;
    int curLen = 0;
    for (const wchar_t* q = p; ; ++q)
    {
        if (*q == L'/' || *q == L'\0')
        {
            // Each abID is represented as hex, so divide by 2
            abidLens[i++] = curLen / 2;
            totalAbidLen += curLen / 2;
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

    // Allocate array of pointers for each abID and a buffer for all abID data
    BYTE** abids = (BYTE**)::CoTaskMemAlloc(itemCount * sizeof(BYTE*));
    if (!abids)
    {
        ::CoTaskMemFree(abidLens);
        return E_OUTOFMEMORY;
    }

    // abidData holds all abID bytes contiguously
    BYTE* abidData = (BYTE*)::CoTaskMemAlloc(totalAbidLen > 0 ? totalAbidLen : 1);
    if (!abidData)
    {
        ::CoTaskMemFree(abidLens);
        ::CoTaskMemFree(abids);
        return E_OUTOFMEMORY;
    }

    int abidDataOffset = 0;

    // Third pass: parse hex string into abID byte arrays
    p = bstrPath;
    for (i = 0; i < itemCount; ++i)
    {
        abids[i] = abidData + abidDataOffset;
        int len = abidLens[i];
        for (int j = 0; j < len; ++j)
        {
            // Validate hex digits
            if (!::iswxdigit(p[0]) || !::iswxdigit(p[1]))
            {
                ::CoTaskMemFree(abidLens);
                ::CoTaskMemFree(abids);
                ::CoTaskMemFree(abidData);
                return E_INVALIDARG;
            }
            // Convert two hex characters to a byte
            wchar_t hex[3] = { p[0], p[1], 0 };
            abids[i][j] = (BYTE)::wcstoul(hex, nullptr, 16);
            p += 2;
        }
        abidDataOffset += len;
        if (*p == L'/')
        {
            ++p;
        }
    }

    // Calculate total size needed for the ITEMIDLIST structure
    // Each SHITEMID: sizeof(USHORT) + abidLen, plus a final terminator
    size_t total = 0;
    for (i = 0; i < itemCount; ++i)
    {
        total += sizeof(USHORT) + abidLens[i];
    }
    total += sizeof(USHORT); // Add space for the terminating SHITEMID

    // Allocate the final ITEMIDLIST buffer
    BYTE* buffer = (BYTE*)::CoTaskMemAlloc(total);
    if (!buffer)
    {
        ::CoTaskMemFree(abidLens);
        ::CoTaskMemFree(abids);
        ::CoTaskMemFree(abidData);
        return E_OUTOFMEMORY;
    }

    // Write each SHITEMID (length + abID bytes) into the buffer
    BYTE* cur = buffer;
    for (i = 0; i < itemCount; ++i)
    {
        USHORT cb = static_cast<USHORT>(sizeof(USHORT) + abidLens[i]);
        *(USHORT*)cur = cb;
        if (abidLens[i] > 0)
        {
            ::memcpy(cur + sizeof(USHORT), abids[i], abidLens[i]);
        }
        cur += cb;
    }

    // Write the terminating SHITEMID (length 0)
    *(USHORT*)cur = 0;
    *ppidl = reinterpret_cast<LPITEMIDLIST>(buffer);

    // Free temporary allocations
    ::CoTaskMemFree(abidLens);
    ::CoTaskMemFree(abids);
    ::CoTaskMemFree(abidData);

    return S_OK;
}
