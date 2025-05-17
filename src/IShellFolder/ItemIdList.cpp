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
        *ppidl = (LPITEMIDLIST)::CoTaskMemAlloc(sizeof(USHORT));
        if (!*ppidl)
        {
            return E_OUTOFMEMORY;
        }
        *(USHORT*)*ppidl = 0;
        return S_OK;
    }

    // First pass: count items and their abID hex lengths
    const wchar_t* p = bstrPath;
    int itemCount = 1;
    int curLen = 0;
    int totalAbidLen = 0;

    // We will do a single pass to count items and their hex lengths
    // To avoid a separate allocation for abidLens, use a stack array if itemCount is small, else allocate
    int abidLensStack[16];
    int* abidLens = abidLensStack;
    int abidLensCapacity = 16;

    // First, count items and their hex lengths
    // We'll do a first pass to count itemCount
    for (const wchar_t* q = p; *q; ++q)
    {
        if (*q == L'/')
        {
            ++itemCount;
        }
    }

    // If more than stack capacity, allocate
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

    // Calculate total size for ITEMIDLIST: each SHITEMID = sizeof(USHORT) + abidLen, plus terminator
    size_t total = 0;
    for (i = 0; i < itemCount; ++i)
    {
        total += sizeof(USHORT) + abidLens[i];
    }
    total += sizeof(USHORT); // terminator

    // Allocate a single buffer for the final ITEMIDLIST and for all abID data
    BYTE* buffer = (BYTE*)::CoTaskMemAlloc(total);
    if (!buffer)
    {
        if (abidLens != abidLensStack)
        {
            ::CoTaskMemFree(abidLens);
        }
        return E_OUTOFMEMORY;
    }

    // Parse hex string directly into the buffer
    BYTE* cur = buffer;
    p = bstrPath;
    for (i = 0; i < itemCount; ++i)
    {
        USHORT cb = static_cast<USHORT>(sizeof(USHORT) + abidLens[i]);
        *(USHORT*)cur = cb;
        BYTE* abID = cur + sizeof(USHORT);
        for (int j = 0; j < abidLens[i]; ++j)
        {
            if (!::iswxdigit(p[0]) || !::iswxdigit(p[1]))
            {
                if (abidLens != abidLensStack)
                {
                    ::CoTaskMemFree(abidLens);
                }
                ::CoTaskMemFree(buffer);
                return E_INVALIDARG;
            }
            wchar_t hex[3] = { p[0], p[1], 0 };
            abID[j] = (BYTE)::wcstoul(hex, nullptr, 16);
            p += 2;
        }
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
