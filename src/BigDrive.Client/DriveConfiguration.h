// <copyright file="DriveConfiguration.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <comutil.h> 

#include "GuidUtil.h"

using namespace BigDriveClient;

class DriveConfiguration
{
public:

    // Properties
    GUID id;
    BSTR name;

    // Constructor
    DriveConfiguration()
        : id(GUID_NULL), name(nullptr)
    {
    }

    DriveConfiguration(LPCWSTR jsonString)
        : id(GUID_NULL), name(nullptr)
    {
        ParseJson(jsonString);
    }

    // Destructor to free allocated memory
    ~DriveConfiguration()
    {
        if (name)
        {
            ::SysFreeString(name);
        }
    }

    // Method to parse the JSON string
    HRESULT ParseJson(LPCWSTR jsonString)
    {
        HRESULT hrReturn = S_OK;

        if (!jsonString)
        {
            return E_INVALIDARG;
        }

        LPCWSTR idKey = L"\"id\":\"";
        LPCWSTR idStart = ::wcsstr(jsonString, idKey);

        LPCWSTR nameKey = L"\"name\":";
        LPCWSTR nameStart = ::wcsstr(jsonString, nameKey);

        BSTR szId = nullptr;

        // Extract "id" value
        if (idStart)
        {
            idStart += ::wcslen(idKey);
            LPCWSTR idEnd = ::wcschr(idStart, L'"');
            if (idEnd)
            {
                size_t idLength = idEnd - idStart;
                szId = ::SysAllocStringLen(idStart, static_cast<UINT>(idLength));

                // Convert string to GUID
                hrReturn = GUIDFromString(szId, &id);
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

    End:

        if(szId)
        {
            ::SysFreeString(szId);
            szId = nullptr;
        }

        return hrReturn;
    }
};
