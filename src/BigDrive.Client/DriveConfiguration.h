// <copyright file="DriveConfiguration.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <comutil.h> 

class DriveConfiguration
{
public:

    // Properties
    BSTR id;
    BSTR name;

    // Constructor
    DriveConfiguration()
        : id(nullptr), name(nullptr)
    {
    }

    DriveConfiguration(LPCWSTR jsonString)
        : id(nullptr), name(nullptr)
    {
        ParseJson(jsonString);
    }

    // Destructor to free allocated memory
    ~DriveConfiguration()
    {
        if (id)
        {
            ::SysFreeString(id);
        }

        if (name)
        {
            ::SysFreeString(name);
        }
    }

    // Method to parse the JSON string
    void ParseJson(LPCWSTR jsonString)
    {
        if (!jsonString)
        {
            return;
        }

        // Extract "id" value
        LPCWSTR idKey = L"\"id\":\"";
        LPCWSTR idStart = ::wcsstr(jsonString, idKey);
        if (idStart)
        {
            idStart += ::wcslen(idKey);
            LPCWSTR idEnd = ::wcschr(idStart, L'"');
            if (idEnd)
            {
                size_t idLength = idEnd - idStart;
                id = ::SysAllocStringLen(idStart, static_cast<UINT>(idLength));
            }
        }

        // Extract "name" value
        LPCWSTR nameKey = L"\"name\":";
        LPCWSTR nameStart = ::wcsstr(jsonString, nameKey);
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
    }

    // Method to display the parsed properties
    void DisplayProperties() const
    {
        ::wprintf(L"ID: %s\n", id ? id : L"(null)");
        ::wprintf(L"Name: %s\n", name ? name : L"(null)");
    }
};
