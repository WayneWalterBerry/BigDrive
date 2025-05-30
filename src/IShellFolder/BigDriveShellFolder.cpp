// <copyright file="BigDriveShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"

BigDriveShellFolderEventLogger BigDriveShellFolder::s_eventLogger(L"BigDrive.ShellFolder");

HRESULT BigDriveShellFolder::GetProviderCLSID(CLSID& clsidProvider) const
{
	return S_OK;
}

HRESULT BigDriveShellFolder::GetPath(BSTR& bstrPath)
{
	bstrPath = ::SysAllocString(L"\\");
	return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::AllocateBigDriveItemId(BigDriveItemType nType, BSTR bstrName, LPITEMIDLIST& pidl)
{
    pidl = nullptr;

    if (!bstrName)
    {
        return E_INVALIDARG;
    }

    // Calculate the size needed for the name (including null terminator)
    size_t nameLen = (SysStringLen(bstrName) + 1) * sizeof(WCHAR);

    // The SHITEMID structure: [cb][nType as int][bstrName][terminator]
    size_t cb = sizeof(USHORT) + sizeof(int) + nameLen;
    BYTE* buffer = (BYTE*)CoTaskMemAlloc(cb + sizeof(USHORT)); // +2 for PIDL terminator

    if (!buffer)
        return E_OUTOFMEMORY;

    // Set cb (size of this SHITEMID, including cb itself)
    *(USHORT*)buffer = static_cast<USHORT>(cb);

    // Set nType as int
    *(int*)(buffer + sizeof(USHORT)) = static_cast<int>(nType);

    // Copy bstrName after nType
    memcpy(buffer + sizeof(USHORT) + sizeof(int), bstrName, nameLen);

    // Add PIDL terminator (USHORT 0) after the SHITEMID
    *(USHORT*)(buffer + cb) = 0;

    pidl = reinterpret_cast<LPITEMIDLIST>(buffer);
    return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetBigDriveItemNameFromPidl(PCUIDLIST_RELATIVE pidl, STRRET* pName)
{
    if (!pidl || !pName)
        return E_INVALIDARG;

    // Initialize output
    ZeroMemory(pName, sizeof(STRRET));

    // Traverse to the last SHITEMID in the PIDL chain
    const BYTE* current = reinterpret_cast<const BYTE*>(pidl);
    const USHORT* cbPtr = reinterpret_cast<const USHORT*>(current);

    // Find the last nonzero cb
    while (*cbPtr != 0)
    {
        current += *cbPtr;
        cbPtr = reinterpret_cast<const USHORT*>(current);
    }

    // Step back to the last item
    if (current == reinterpret_cast<const BYTE*>(pidl))
        return E_FAIL;

    // Go back to the start of the last SHITEMID
    const BYTE* last = reinterpret_cast<const BYTE*>(pidl);
    const BYTE* next = last;
    while (true)
    {
        USHORT cb = *(reinterpret_cast<const USHORT*>(next));
        if (cb == 0)
            break;
        last = next;
        next += cb;
    }

    // Layout: [USHORT cb][int nType][WCHAR szName[]]
    const BYTE* p = last;
    USHORT cb = *(reinterpret_cast<const USHORT*>(p));
    if (cb < sizeof(USHORT) + sizeof(int) + sizeof(WCHAR))
        return E_FAIL; // Not enough data

    // Name starts after cb and nType
    const WCHAR* szName = reinterpret_cast<const WCHAR*>(p + sizeof(USHORT) + sizeof(int));

    // Defensive: ensure null-terminated
    size_t maxChars = (cb - sizeof(USHORT) - sizeof(int)) / sizeof(WCHAR);
    size_t len = 0;
    for (; len < maxChars; ++len)
    {
        if (szName[len] == L'\0')
            break;
    }
    if (len == maxChars)
        return E_FAIL; // Not null-terminated

    // Allocate and copy the name for STRRET_WSTR
    size_t nameBytes = (len + 1) * sizeof(WCHAR);
    LPWSTR pOut = (LPWSTR)CoTaskMemAlloc(nameBytes);
    if (!pOut)
        return E_OUTOFMEMORY;
    memcpy(pOut, szName, nameBytes);

    pName->uType = STRRET_WSTR;
    pName->pOleStr = pOut;

    return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::WriteError(LPCWSTR szMessage)
{
    wchar_t guidStr[64];

    // Format: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    swprintf(guidStr, 64,
        L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        m_driveGuid.Data1, m_driveGuid.Data2, m_driveGuid.Data3,
        m_driveGuid.Data4[0], m_driveGuid.Data4[1],
        m_driveGuid.Data4[2], m_driveGuid.Data4[3], m_driveGuid.Data4[4], m_driveGuid.Data4[5], m_driveGuid.Data4[6], m_driveGuid.Data4[7]);

	s_eventLogger.WriteErrorFormmated(L"%s for drive: %s", szMessage, guidStr);

    return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::WriteErrorFormatted(LPCWSTR formatter, ...)
{
    wchar_t formattedMsg[1024];
    va_list args;
    va_start(args, formatter);
    _vsnwprintf_s(formattedMsg, _countof(formattedMsg), _TRUNCATE, formatter, args);
    va_end(args);

    wchar_t guidStr[64];

    // Format: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    swprintf(guidStr, 64,
        L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        m_driveGuid.Data1, m_driveGuid.Data2, m_driveGuid.Data3,
        m_driveGuid.Data4[0], m_driveGuid.Data4[1],
        m_driveGuid.Data4[2], m_driveGuid.Data4[3], m_driveGuid.Data4[4], m_driveGuid.Data4[5], m_driveGuid.Data4[6], m_driveGuid.Data4[7]);

    wchar_t finalMsg[1200];

    _snwprintf_s(finalMsg, _countof(finalMsg), _TRUNCATE, L"%s for drive: %s", formattedMsg, guidStr);

	s_eventLogger.WriteError(finalMsg);

    return S_OK;
}