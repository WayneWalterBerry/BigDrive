// <copyright file="GuidUtil.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "pch.h"

#include <wtypes.h>
#include <string>
#include <combaseapi.h>

namespace BigDriveClient
{
    inline HRESULT StringFromGUID(const GUID& guid, wchar_t* guidWithoutBraces, size_t bufferSize);
    inline HRESULT GUIDFromString(const wchar_t* wideStr, GUID* pGuid);
    
    HRESULT StringFromGUID(const GUID& guid, wchar_t* guidWithBraces, size_t bufferSize)
    {
        // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
        wchar_t guidString[39];
        if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
        {
            return E_FAIL;
        }

        // Ensure the buffer is large enough
        if (bufferSize < wcslen(guidString) + 1) // Add 1 for null terminator
        {
            return E_FAIL;
        }

        // Copy the GUID string with braces directly
        wcsncpy_s(guidWithBraces, bufferSize, guidString, wcslen(guidString));
        guidWithBraces[wcslen(guidString)] = L'\0'; // Null-terminate the string

        return S_OK;
    }


    HRESULT GUIDFromString(const wchar_t* wideStr, GUID* pGuid)
    {
        if (!wideStr || !pGuid)
        {
            // Return error if input is null
            return E_INVALIDARG;  
        }

        // Parse GUID components manually
        int result = swscanf_s(wideStr, L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
            &pGuid->Data1, &pGuid->Data2, &pGuid->Data3,
            &pGuid->Data4[0], &pGuid->Data4[1], &pGuid->Data4[2], &pGuid->Data4[3],
            &pGuid->Data4[4], &pGuid->Data4[5], &pGuid->Data4[6], &pGuid->Data4[7]);

        return (result == 11) ? S_OK : E_FAIL;  // Ensure all parts were parsed
    }
}