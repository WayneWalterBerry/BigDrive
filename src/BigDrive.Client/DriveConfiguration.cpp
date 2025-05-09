// <copyright file="DriveConfiguration.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <string>

// Header
#include "DriveConfiguration.h"

using namespace BigDriveClient;

/// <summary>
/// Parses a JSON string to populate the drive configuration properties.
/// </summary>
/// <param name="jsonString">The JSON string containing the drive configuration.</param>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT DriveConfiguration::ParseJson(LPCWSTR jsonString)
{
    HRESULT hrReturn = S_OK;

    if (!jsonString)
    {
        return E_INVALIDARG;
    }

    // Check if jsonString is an empty string
    if (::wcslen(jsonString) == 0)
    {
        return E_INVALIDARG; // Return an error for empty strings
    }

    LPCWSTR idKey = L"\"id\":\"";
    LPCWSTR idStart = ::wcsstr(jsonString, idKey);

    LPCWSTR nameKey = L"\"name\":";
    LPCWSTR nameStart = ::wcsstr(jsonString, nameKey);

    LPCWSTR clsidKey = L"\"clsid\":\"";
    LPCWSTR clsidStart = ::wcsstr(jsonString, clsidKey);

    BSTR szId = nullptr;

    // Extract "id" value
    if (idStart)
    {
        idStart += ::wcslen(idKey);
        LPCWSTR idEnd = ::wcschr(idStart, L'"');
        if (idEnd)
        {
            size_t idLength = idEnd - idStart;

            // Allocate a single BSTR with space for the brackets, JSON formatting
            // for GUIDs doesn't include the brackets, but GUIDFromString requires
            // the brackets to be present in the string.
            BSTR szIdWithBrackets = ::SysAllocStringLen(nullptr, static_cast<UINT>(idLength + 2));
            if (szIdWithBrackets)
            {
                szIdWithBrackets[0] = L'{';
                ::wcsncpy_s(szIdWithBrackets + 1, idLength + 1, idStart, idLength);
                szIdWithBrackets[idLength + 1] = L'}';
                szIdWithBrackets[idLength + 2] = L'\0';

                // Pass the modified string to GUIDFromString
                hrReturn = GUIDFromString(szIdWithBrackets, &id);

                // Free the allocated memory for szIdWithBrackets
                ::SysFreeString(szIdWithBrackets);
                szIdWithBrackets = nullptr;
            }
            else
            {
                hrReturn = E_OUTOFMEMORY;
            }

            if (FAILED(hrReturn))
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
                hrReturn = GUIDFromString(szClsidWithBrackets, &clsid);

                // Free the allocated memory for szClsidWithBrackets
                ::SysFreeString(szClsidWithBrackets);
                szClsidWithBrackets = nullptr;
            }
            else
            {
                hrReturn = E_OUTOFMEMORY;
            }

            if (FAILED(hrReturn))
            {
                goto End;
            }
        }
    }

End:

    if (szId)
    {
        ::SysFreeString(szId);
        szId = nullptr;
    }

    return hrReturn;
}
