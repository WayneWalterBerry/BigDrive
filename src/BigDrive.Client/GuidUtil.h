// <copyright file="GuidUtil.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <wtypes.h>
#include <string>

namespace BigDriveClient
{
    inline HRESULT StringFromGUID(const GUID& guid, wchar_t* guidWithoutBraces, size_t bufferSize);
    inline HRESULT GUIDFromString(const wchar_t* wideStr, GUID* pGuid);
    
    /// <summary>
    /// Converts a GUID to a string without curly braces.
    /// <param name="guid">The GUID to convert.</param>
    /// <param name="guidWithoutBraces">The output buffer for the GUID string without braces.</param>
    /// <param name="bufferSize">The size of the output buffer.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT StringFromGUID(const GUID& guid, wchar_t* guidWithoutBraces, size_t bufferSize)
    {
        // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
        wchar_t guidString[39];
        if (::StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
        {
            return E_FAIL;
        }

        // Remove the leading '{' and trailing '}'
        size_t guidLen = wcslen(guidString);
        if (guidLen < 2)
        {
            return E_FAIL;
        }

        // The string without braces is 36 characters
        if (bufferSize < 37) // 36 chars + null terminator
        {
            return E_FAIL;
        }

        // Copy the substring without the first and last character
        wcsncpy_s(guidWithoutBraces, bufferSize, guidString + 1, 36);
        guidWithoutBraces[36] = L'\0'; // Null-terminate the string

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