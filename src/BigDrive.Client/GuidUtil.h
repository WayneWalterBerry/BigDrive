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
    
    HRESULT StringFromGUID(const GUID& guid, wchar_t* guidWithoutBraces, size_t bufferSize)
    {

        // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
        wchar_t guidString[39];
        if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
        {
            return E_FAIL;
        }

        // Ensure the buffer is large enough
        if (bufferSize < wcslen(guidString) - 2 + 1) // Remove 2 for braces and add 1 for null terminator
        {
            return E_FAIL;
        }

        // Remove the braces from the GUID string
        if (guidString[0] == L'{' && guidString[wcslen(guidString) - 1] == L'}')
        {
            wcsncpy_s(guidWithoutBraces, bufferSize, guidString + 1, wcslen(guidString) - 2);
            guidWithoutBraces[wcslen(guidString) - 2] = L'\0'; // Null-terminate the string
        }
        else
        {
            wcsncpy_s(guidWithoutBraces, bufferSize, guidString, wcslen(guidString));
            guidWithoutBraces[wcslen(guidString)] = L'\0'; // Null-terminate the string
        }

        return S_OK;
    }

    HRESULT GUIDFromString(const wchar_t* wideStr, GUID* pGuid)
    {
        if (!wideStr || !pGuid)
            return E_INVALIDARG;  // Return error if input is null

        // Parse GUID components manually
        int result = swscanf_s(wideStr, L"%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
            &pGuid->Data1, &pGuid->Data2, &pGuid->Data3,
            &pGuid->Data4[0], &pGuid->Data4[1], &pGuid->Data4[2], &pGuid->Data4[3],
            &pGuid->Data4[4], &pGuid->Data4[5], &pGuid->Data4[6], &pGuid->Data4[7]);

        return (result == 11) ? S_OK : E_FAIL;  // Ensure all parts were parsed
    }
}