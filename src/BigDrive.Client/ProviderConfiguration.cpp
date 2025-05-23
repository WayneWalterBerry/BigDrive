// <copyright file="ProviderConfiguration.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <string>

// Header
#include "ProviderConfiguration.h"

using namespace BigDriveClient;

/// <summary>
/// Parses a JSON string to populate the drive configuration properties.
/// </summary>
/// <param name="jsonString">The JSON string containing the drive configuration.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT ProviderConfiguration::ParseJson(LPCWSTR jsonString)
{
    HRESULT hr = S_OK;

    if (!jsonString)
    {
        return E_INVALIDARG;
    }

    // Check if jsonString is an empty string
    if (::wcslen(jsonString) == 0)
    {
        return E_INVALIDARG; // Return an error for empty strings
    }

    LPCWSTR clsidKey = L"\"clsid\":\"";
    LPCWSTR clsidStart = ::wcsstr(jsonString, clsidKey);

    LPCWSTR nameKey = L"\"name\":";
    LPCWSTR nameStart = ::wcsstr(jsonString, nameKey);

    // Extract "clsid" value
    if (clsidStart)
    {
        clsidStart += ::wcslen(clsidKey);
        LPCWSTR clsidEnd = ::wcschr(clsidStart, L'"');
        if (clsidEnd)
        {
            size_t clsidLength = clsidEnd - clsidStart;

            // Allocate a single BSTR with space for the brackets
            BSTR szClsidWithBrackets = ::SysAllocStringLen(nullptr, static_cast<UINT>(clsidLength + 2));
            if (szClsidWithBrackets)
            {
                szClsidWithBrackets[0] = L'{';
                ::wcsncpy_s(szClsidWithBrackets + 1, clsidLength + 1, clsidStart, clsidLength);
                szClsidWithBrackets[clsidLength + 1] = L'}';
                szClsidWithBrackets[clsidLength + 2] = L'\0';

                // Pass the modified string to GUIDFromString
                hr = GUIDFromString(szClsidWithBrackets, &clsid);

                // Free the allocated memory for szClsidWithBrackets
                ::SysFreeString(szClsidWithBrackets);
                szClsidWithBrackets = nullptr;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (FAILED(hr))
            {
                goto End;
            }
        }
    }

    // Extract "name" value
    if (nameStart)
    {
        nameStart += ::wcslen(nameKey);
        if (::wcsncmp(nameStart, L"null", 4) == 0)
        {
            name = ::SysAllocString(L"null");
        }
        else if (*nameStart == L'"')
        {
            nameStart++;
            LPCWSTR nameEnd = ::wcschr(nameStart, L'"');
            if (nameEnd)
            {
                size_t nameLength = nameEnd - nameStart;
                name = ::SysAllocStringLen(nameStart, static_cast<UINT>(nameLength));
            }
        }
    }


End:

    return hr;
}
