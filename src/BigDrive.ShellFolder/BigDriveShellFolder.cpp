// <copyright file="BigDriveShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "BigDriveShellFolder.h"

// Local
#include "LaunchDebugger.h"
#include "BigDriveShellFolderStatic.h"
#include "..\BigDrive.Client\BigDriveInterfaceProvider.h"
#include "..\BigDrive.Client\BigDriveConfigurationClient.h"

#include <oleauto.h> 
#include <shlguid.h>
#include <shlwapi.h>

#ifndef PID_STG_NAME
#define PID_STG_NAME 10
#endif

#ifndef PID_STG_WRITETIME
#define PID_STG_WRITETIME 14
#endif

#ifndef PID_STG_SIZE
#define PID_STG_SIZE 12
#endif

BigDriveShellFolderStatic BigDriveShellFolder::s_staticData;
BigDriveShellFolderEventLogger BigDriveShellFolder::s_eventLogger(L"BigDrive.ShellFolder");

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetProviderCLSID(CLSID& clsidProvider) const
{
	return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetPathForProviders(LPCITEMIDLIST pidl, BSTR& bstrPath)
{
	int nSkip = 0; 

    // Start with the initial '\'
    WCHAR szPath[MAX_PATH] = L"\\";
    size_t cchPath = 1;

    if (s_staticData.IsPidlRootedAtMyComputer(pidl))
    {
        // Skip My Computer and GUID PIDLs
        nSkip = nSkip + 2;
    }

    const BYTE* p = reinterpret_cast<const BYTE*>(pidl);
    int pidlIndex = 0;

    // Traverse the PIDL chain
    while (true)
    {
        USHORT cb = *(reinterpret_cast<const USHORT*>(p));
        if (cb == 0)
        {
            break;
        }

        if (pidlIndex >= nSkip)
        {
            if (!IsValidBigDriveItemId(reinterpret_cast<PCUIDLIST_RELATIVE>(p)))
            {
                // Not a valid BigDrive PIDL
                return E_FAIL;
            }

            const WCHAR* szName = reinterpret_cast<const WCHAR*>(p + sizeof(USHORT) + sizeof(int));

            // Defensive: ensure null-terminated
            size_t maxChars = (cb - sizeof(USHORT) - sizeof(int)) / sizeof(WCHAR);
            size_t len = 0;

            for (; len < maxChars; ++len)
            {
                if (szName[len] == L'\0')
                {
                    break;
                }
            }

            if (len == 0 || len == maxChars)
            {
                // Not null-terminated or empty
                return E_FAIL;
            }

            // Add '\' if not the first component after root
            if (cchPath > 1 && cchPath < MAX_PATH - 1)
            {
                szPath[cchPath++] = L'\\';
            }

            // Copy szName to szPath
            size_t i = 0;
            for (; i < len && cchPath < MAX_PATH - 1; ++i)
            {
                szPath[cchPath++] = szName[i];
            }
            szPath[cchPath] = L'\0';
        }

        p += cb;
        ++pidlIndex;
    }

    bstrPath = ::SysAllocString(szPath);
    if (!bstrPath)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetPathForLogging(CLSID driveGuid, LPCITEMIDLIST pidl, BSTR& bstrPath)
{
	int nSkip = 0; // Number of PIDLs to skip
    bstrPath = nullptr;

    const WCHAR szRootDelimiter[3] = L"\\\\";

    if (pidl == nullptr)
    {
        bstrPath = ::SysAllocString(szRootDelimiter);
        return S_OK;
    }

    WCHAR szPath[MAX_PATH] = L"";
    size_t cchPath = 0;

    if (s_staticData.IsPidlRootedAtMyComputer(pidl))
    {
        // Root Delimiter
        ::wcsncat_s(szPath, MAX_PATH, szRootDelimiter, _TRUNCATE);
        cchPath += wcslen(szRootDelimiter);

        // My PC
        LPCWSTR szComputeName = s_staticData.GetMyComputerName();

        ::wcsncat_s(szPath, MAX_PATH, szComputeName, _TRUNCATE);
        cchPath += wcslen(szComputeName);

        // Add the backslash after My PC
        ::wcsncat_s(szPath, MAX_PATH, L"\\", _TRUNCATE);
        cchPath += 1;

        // Skip My PC PIDL
        nSkip++;

        WCHAR guidStr[64];
        int guidLen = swprintf(guidStr, 64,
            L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
            driveGuid.Data1, driveGuid.Data2, driveGuid.Data3,
            driveGuid.Data4[0], driveGuid.Data4[1],
            driveGuid.Data4[2], driveGuid.Data4[3], driveGuid.Data4[4],
            driveGuid.Data4[5], driveGuid.Data4[6], driveGuid.Data4[7]);

        if (guidLen > 0 && cchPath + guidLen < MAX_PATH) {
            wcsncat_s(szPath, MAX_PATH, guidStr, _TRUNCATE);
            cchPath += guidLen;
        }

        // Skip the GUID PIDL
        nSkip++;
    }

    const BYTE* p = reinterpret_cast<const BYTE*>(pidl);
    int pidlIndex = 0;

    // Traverse the PIDL chain
    while (true)
    {
        USHORT cb = *(reinterpret_cast<const USHORT*>(p));
        if (cb == 0)
        {
            break;
        }

        if (pidlIndex >= nSkip)
        {
            if (!IsValidBigDriveItemId(reinterpret_cast<PCUIDLIST_RELATIVE>(p)))
            {
				// Not a valid BigDrive PIDL
                return E_FAIL;
			}

            const WCHAR* szName = reinterpret_cast<const WCHAR*>(p + sizeof(USHORT) + sizeof(int));

            // Defensive: ensure null-terminated
            size_t maxChars = (cb - sizeof(USHORT) - sizeof(int)) / sizeof(WCHAR);
            size_t len = 0;

            for (; len < maxChars; ++len)
            {
                if (szName[len] == L'\0')
                {
                    break;
                }
            }

            if (len == 0 || len == maxChars)
            {
                // Not null-terminated or empty
                return E_FAIL;
            }

            // Add '\' if not the first component after root
            if (cchPath > 1 && cchPath < MAX_PATH - 1)
            {
                szPath[cchPath++] = L'\\';
            }

            // Copy szName to szPath
            size_t i = 0;
            for (; i < len && cchPath < MAX_PATH - 1; ++i)
            {
                szPath[cchPath++] = szName[i];
            }
            szPath[cchPath] = L'\0';
        }

        p += cb;
        ++pidlIndex;
    }

    bstrPath = ::SysAllocString(szPath);
    if (!bstrPath)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::AllocBigDrivePidl(BigDriveItemType nType, BSTR bstrPath, LPITEMIDLIST& ppidl)
{
    if (!bstrPath)
    {
        return E_INVALIDARG;
    }

    ppidl = nullptr;

    LPWSTR szPath = bstrPath;

    if (szPath[0] == L'\\')
    {
        ++szPath;
    }

    const wchar_t* p = szPath;

    // Count components
    int cComponents = 0;

    while (*p)
    {
        ++cComponents;
        // Find next backslash or end
        while (*p && *p != L'\\') ++p;
        if (*p == L'\\') ++p;
    }

    if (cComponents == 0)
    {
        return E_INVALIDARG;
    }

    // Allocate array of pointers for component strings
    wchar_t** componentPtrs = (wchar_t**)CoTaskMemAlloc(sizeof(wchar_t*) * cComponents);
    int* componentLens = (int*)CoTaskMemAlloc(sizeof(int) * cComponents);
    if (!componentPtrs || !componentLens)
    {
        if (componentPtrs) CoTaskMemFree(componentPtrs);
        if (componentLens) CoTaskMemFree(componentLens);
        return E_OUTOFMEMORY;
    }

    // Split path into components (in-place, no std::)
    int idx = 0;
    p = szPath;
    while (*p && idx < cComponents)
    {
        componentPtrs[idx] = (wchar_t*)p;
        int len = 0;
        while (p[len] && p[len] != L'\\') ++len;
        componentLens[idx] = len;
        p += len;
        if (*p == L'\\') ++p;
        ++idx;
    }

    // Calculate total size for all SHITEMIDs
    SIZE_T totalSize = 0;
    for (int i = 0; i < cComponents; ++i)
    {
        SIZE_T nameLen = (componentLens[i] + 1) * sizeof(wchar_t);
        SIZE_T cb = sizeof(USHORT) + sizeof(UINT) + nameLen;
        totalSize += cb;
    }
    totalSize += sizeof(USHORT); // zero terminator

    BYTE* pidlMem = (BYTE*)CoTaskMemAlloc(totalSize);
    if (!pidlMem)
    {
        ::CoTaskMemFree(componentPtrs);
        ::CoTaskMemFree(componentLens);
        return E_OUTOFMEMORY;
    }

    // Fill each SHITEMID
    BYTE* dest = pidlMem;
    for (int i = 0; i < cComponents; ++i)
    {
        SIZE_T nameLen = (componentLens[i] + 1) * sizeof(wchar_t);
        SIZE_T cb = sizeof(USHORT) + sizeof(UINT) + nameLen;
        USHORT* pcb = (USHORT*)dest;
        *pcb = (USHORT)cb;
        UINT* puType = (UINT*)(dest + sizeof(USHORT));
        *puType = (i == cComponents - 1) ? (UINT)nType : (UINT)BigDriveItemType_Folder;
        wchar_t* pszName = (wchar_t*)(dest + sizeof(USHORT) + sizeof(UINT));
        for (int j = 0; j < componentLens[i]; ++j)
        {
            pszName[j] = componentPtrs[i][j];
        }

        pszName[componentLens[i]] = L'\0';
        dest += cb;
    }

    // Add zero terminator
    *((USHORT*)dest) = 0;

    ppidl = reinterpret_cast<LPITEMIDLIST>(pidlMem);

    ::CoTaskMemFree(componentPtrs);
    ::CoTaskMemFree(componentLens);

    return S_OK;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetBigDriveItemNameFromPidl(PCUITEMID_CHILD pidl, STRRET* pName)
{
    if (!pidl || !pName)
    {
        return E_INVALIDARG;
    }

    // Initialize output
    ::ZeroMemory(pName, sizeof(STRRET));

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
        {
            break;
        }
    }

    if (len == maxChars)
    {
        return E_FAIL; // Not null-terminated
    }

    // Allocate and copy the name for STRRET_WSTR
    size_t nameBytes = (len + 1) * sizeof(WCHAR);
    LPWSTR pOut = (LPWSTR)::CoTaskMemAlloc(nameBytes);

    if (!pOut)
    {
        return E_OUTOFMEMORY;
    }

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

/// <inheritdoc />
bool BigDriveShellFolder::IsValidBigDriveItemId(PCUIDLIST_RELATIVE pidl)
{
    if (!pidl)
    {
        return false;
    }

    // Minimum size: [USHORT cb][UINT uType][at least one WCHAR for szName + null]
    const USHORT cb = *(const USHORT*)pidl;
    if (cb < sizeof(USHORT) + sizeof(UINT) + sizeof(WCHAR))
    {
        return false;
    }

    // Check uType is a known value
    const BYTE* p = reinterpret_cast<const BYTE*>(pidl);
    UINT uType = *(const UINT*)(p + sizeof(USHORT));
    if (uType != BigDriveItemType_File && uType != BigDriveItemType_Folder)
    {
        return false;
    }

    // szName points just after [USHORT][UINT]
    const WCHAR* szName = reinterpret_cast<const WCHAR*>(p + sizeof(USHORT) + sizeof(UINT));
    if (!szName || szName[0] == L'\0')
    {
        return false;
    }

    // Defensive: ensure null-terminated within bounds
    size_t maxChars = (cb - sizeof(USHORT) - sizeof(UINT)) / sizeof(WCHAR);
    size_t i = 0;
    for (; i < maxChars; ++i)
    {
        if (szName[i] == L'\0')
        {
            return true;
        }
    }

    return false;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::VariantToStrRet(VARIANT& var, STRRET* pStrRet)
{
    WCHAR szBuf[MAX_PATH];
    SYSTEMTIME st;
    HRESULT hr = S_OK;

    if (!pStrRet)
    {
        return E_POINTER;
    }

    // Handle DATE for date columns
    if (var.vt == VT_DATE)
    {
        if (VariantTimeToSystemTime(var.date, &st)) 
        {
            // Format the date/time according to system locale
            ::GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szBuf, MAX_PATH);

            // Allocate memory for the string
            LPWSTR pszStr = (LPWSTR)::CoTaskMemAlloc((wcslen(szBuf) + 1) * sizeof(WCHAR));
            if (!pszStr)
            {
                return E_OUTOFMEMORY;
            }

            wcscpy_s(pszStr, wcslen(szBuf) + 1, szBuf);
            pStrRet->uType = STRRET_WSTR;
            pStrRet->pOleStr = pszStr;
            return S_OK;
        }

        return E_FAIL;
    }
    else if (var.vt == VT_UI8)
    {
        ULONGLONG size = var.ullVal;

        // Format the size in a human-readable way (KB, MB, GB)
        if (size < 1024) // Bytes
        {
            swprintf_s(szBuf, MAX_PATH, L"%llu bytes", size);
        }
        else if (size < 1024 * 1024) // KB
        {
            double kb = size / 1024.0;
            swprintf_s(szBuf, MAX_PATH, L"%.1f KB", kb);
        }
        else if (size < 1024 * 1024 * 1024) // MB
        {
            double mb = size / (1024.0 * 1024.0);
            swprintf_s(szBuf, MAX_PATH, L"%.1f MB", mb);
        }
        else // GB
        {
            double gb = size / (1024.0 * 1024.0 * 1024.0);
            swprintf_s(szBuf, MAX_PATH, L"%.2f GB", gb);
        }

        // Allocate memory for the string
        LPWSTR pszStr = (LPWSTR)::CoTaskMemAlloc((wcslen(szBuf) + 1) * sizeof(WCHAR));
        if (!pszStr)
            return E_OUTOFMEMORY;

        wcscpy_s(pszStr, wcslen(szBuf) + 1, szBuf);
        pStrRet->uType = STRRET_WSTR;
        pStrRet->pOleStr = pszStr;
        return S_OK;
    }

    // Handle other VARIANT types if needed
    // ...

    return E_NOTIMPL;
}

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetShellDetailsProperty(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
    HRESULT hr = S_OK;
    STRRET strret = { 0 };
    DriveConfiguration driveConfiguration;
    BigDriveInterfaceProvider* pInterfaceProvider = nullptr;
    IBigDriveFileInfo* pBigDriveFileInfo = nullptr;
    BSTR bstrPath = nullptr;
    PIDLIST_ABSOLUTE pidlAbsolute = nullptr;
    ULONGLONG ullFileSize = 0;
    DATE dtLastModifiedTime = 0;

    if (!pidl || !pscid || !pv)
    {
        hr = E_INVALIDARG;
        goto End;
    }

    ::VariantInit(pv);

    switch (pscid->pid)
    {
    case 0: // Name
        hr = GetBigDriveItemNameFromPidl(pidl, &strret);
        if (FAILED(hr))
        {
            goto End;
        }
        {
            WCHAR szName[MAX_PATH] = { 0 };
            hr = ::StrRetToBufW(&strret, pidl, szName, ARRAYSIZE(szName));
            if (FAILED(hr))
            {
                goto End;
            }
            pv->vt = VT_BSTR;
            pv->bstrVal = ::SysAllocString(szName);
            if (!pv->bstrVal)
            {
                hr = E_OUTOFMEMORY;
                goto End;
            }
        }
        break;

    case 2: // Size
    {
        const BIGDRIVE_ITEMID* pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(pidl);
        if (pItem && pItem->uType == BigDriveItemType_Folder)
        {
            pv->vt = VT_EMPTY;
            hr = S_OK;
            goto End;
        }
        hr = BigDriveConfigurationClient::GetDriveConfiguration(m_driveGuid, driveConfiguration);
        if (FAILED(hr))
        {
            goto End;
        }
        pInterfaceProvider = new BigDriveInterfaceProvider(driveConfiguration);
        if (!pInterfaceProvider)
        {
            hr = E_OUTOFMEMORY;
            goto End;
        }
        hr = pInterfaceProvider->GetIBigDriveFileInfo(&pBigDriveFileInfo);
        if (FAILED(hr) || !pBigDriveFileInfo)
        {
            goto End;
        }
        pidlAbsolute = ::ILCombine(m_pidlAbsolute, pidl);
        hr = GetPathForProviders(pidlAbsolute, bstrPath);
        if (FAILED(hr))
        {
            goto End;
        }
        hr = pBigDriveFileInfo->GetFileSize(m_driveGuid, bstrPath, &ullFileSize);
        if (FAILED(hr))
        {
            goto End;
        }
        pv->vt = VT_UI8;
        pv->ullVal = ullFileSize;
        break;
    }

    case 3: // Date Modified
        hr = BigDriveConfigurationClient::GetDriveConfiguration(m_driveGuid, driveConfiguration);
        if (FAILED(hr))
        {
            goto End;
        }
        pInterfaceProvider = new BigDriveInterfaceProvider(driveConfiguration);
        if (!pInterfaceProvider)
        {
            hr = E_OUTOFMEMORY;
            goto End;
        }
        hr = pInterfaceProvider->GetIBigDriveFileInfo(&pBigDriveFileInfo);
        if (FAILED(hr) || !pBigDriveFileInfo)
        {
            goto End;
        }
        pidlAbsolute = ::ILCombine(m_pidlAbsolute, pidl);
        hr = GetPathForProviders(pidlAbsolute, bstrPath);
        if (FAILED(hr))
        {
            goto End;
        }
        hr = pBigDriveFileInfo->LastModifiedTime(m_driveGuid, bstrPath, &dtLastModifiedTime);
        if (FAILED(hr))
        {
            goto End;
        }
        pv->vt = VT_DATE;
        pv->date = dtLastModifiedTime;
        break;

    case 9: // Attributes (FMTID_ShellDetails, pid 9)
    {
        const BIGDRIVE_ITEMID* pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(pidl);
        if (pItem == nullptr)
        {
            hr = E_INVALIDARG;
            goto End;
        }

        pv->vt = VT_BSTR;

        if (pItem->uType == BigDriveItemType_Folder)
        {
            pv->bstrVal = ::SysAllocString(L"D");
        }
        else if (pItem->uType == BigDriveItemType_File)
        {
            pv->bstrVal = ::SysAllocString(L"A");
        }
        else
        {
            pv->bstrVal = nullptr;
            hr = E_NOTIMPL;
            goto End;
        }

        if (pv->bstrVal == nullptr)
        {
            hr = E_OUTOFMEMORY;
            goto End;
        }

        hr = S_OK;
        break;
    }

    case 11: // Owner (FMTID_ShellDetails, pid 11)
    {
        // If you have a way to get the owner, set it here.
        // For now, return VT_EMPTY or E_NOTIMPL if not supported.
        pv->vt = VT_EMPTY;
        hr = S_OK;
        break;
    }

    default:
        hr = E_NOTIMPL;
        break;
    }

End:

    if (pidlAbsolute)
    {
        ::ILFree(pidlAbsolute);
        pidlAbsolute = nullptr;
    }

    if (bstrPath)
    {
        ::SysFreeString(bstrPath);
        bstrPath = nullptr;
    }

    if (pInterfaceProvider)
    {
        delete pInterfaceProvider;
        pInterfaceProvider = nullptr;
    }

    if (pBigDriveFileInfo)
    {
        pBigDriveFileInfo->Release();
        pBigDriveFileInfo = nullptr;
    }

    return hr;

}

/// <inheritdoc />
HRESULT BigDriveShellFolder::GetStorageProperty(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
    HRESULT hr = E_NOTIMPL;
    DriveConfiguration driveConfiguration;
    BigDriveInterfaceProvider* pInterfaceProvider = nullptr;
    IBigDriveFileInfo* pBigDriveFileInfo = nullptr;
    BSTR bstrPath = nullptr;
    DATE dtLastModifiedTime;
    PIDLIST_ABSOLUTE pidlAbsolute = nullptr;
    ULONGLONG ullFileSize;
    const BIGDRIVE_ITEMID* pItem = nullptr;

    // Initialize common components needed for both properties
    switch (pscid->pid)
    {
    case PID_STG_WRITETIME:
    case PID_STG_SIZE:

        hr = BigDriveConfigurationClient::GetDriveConfiguration(m_driveGuid, driveConfiguration);
        if (FAILED(hr))
        {
            WriteErrorFormatted(L"GetDetailsEx: Failed to get drive configuration. HRESULT: 0x%08X", hr);
            goto End;
        }

        pInterfaceProvider = new BigDriveInterfaceProvider(driveConfiguration);
        if (pInterfaceProvider == nullptr)
        {
            WriteError(L"GetDetailsEx: Failed to create BigDriveInterfaceProvider");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        hr = pInterfaceProvider->GetIBigDriveFileInfo(&pBigDriveFileInfo);
        switch (hr)
        {
        case S_OK:
            break;
        case S_FALSE:
            // Interface isn't Implemented By The Provider
            goto End;
        default:
            WriteErrorFormatted(L"GetDetailsEx: Failed to obtain IBigDriveFileInfo, HRESULT: 0x%08X", hr);
            break;
        }

        if (pBigDriveFileInfo == nullptr)
        {
            hr = E_FAIL;
            WriteErrorFormatted(L"GetDetailsEx: Failed to obtain IBigDriveFileInfo, HRESULT: 0x%08X", hr);
            goto End;
        }

        pidlAbsolute = ::ILCombine(m_pidlAbsolute, pidl);

        hr = GetPathForProviders(pidlAbsolute, bstrPath);
        if (FAILED(hr))
        {
            goto End;
        }

        break;
    }

    // Speific Code For Each Column
    switch (pscid->pid)
    {
    case  PID_STG_WRITETIME:

        hr = pBigDriveFileInfo->LastModifiedTime(m_driveGuid, bstrPath, &dtLastModifiedTime);
        if (FAILED(hr))
        {
            goto End;
        }

        // Set VARIANT to FILETIME (corrected)
        pv->vt = VT_DATE;
        pv->date = dtLastModifiedTime;

        hr = S_OK;

        break;

    case PID_STG_SIZE:

        // Check if this is a folder
        pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(pidl);
        if (pItem && pItem->uType == BigDriveItemType_Folder)
        {
            // Don't display size for folders
            pv->vt = VT_EMPTY;
            hr = S_OK;
            goto End;
        }

        // Get the file size from our provider
        hr = pBigDriveFileInfo->GetFileSize(m_driveGuid, bstrPath, &ullFileSize);
        if (FAILED(hr))
        {
            goto End;
        }

        // Set VARIANT to 64-bit unsigned integer
        pv->vt = VT_UI8;
        pv->ullVal = ullFileSize;
        hr = S_OK;

        break;

    case 24: // Content Type (FMTID_Storage, PID_STG_CONTENTTYPE)
    {
        pItem = reinterpret_cast<const BIGDRIVE_ITEMID*>(pidl);
        if (pItem == nullptr)
        {
            hr = E_INVALIDARG;
            goto End;
        }

        pv->vt = VT_BSTR;

        if (pItem->uType == BigDriveItemType_Folder)
        {
            pv->bstrVal = ::SysAllocString(L"File Folder");
        }
        else if (pItem->uType == BigDriveItemType_File)
        {
            // Example: Use file extension to determine type
            // For a real implementation, you might want to map extensions to friendly names
            // Here, we just return "File" or you can expand this logic as needed

            // Find the extension in the name
            const WCHAR* szName = pItem->szName;
            const WCHAR* pExt = nullptr;
            for (const WCHAR* p = szName; *p; ++p)
            {
                if (*p == L'.')
                {
                    pExt = p;
                }
            }

            if (pExt && *(pExt + 1))
            {
                // Example: ".txt" => "Text Document"
                if (::lstrcmpiW(pExt, L".txt") == 0)
                {
                    pv->bstrVal = ::SysAllocString(L"Text Document");
                }
                else if (::lstrcmpiW(pExt, L".jpg") == 0 || ::lstrcmpiW(pExt, L".jpeg") == 0)
                {
                    pv->bstrVal = ::SysAllocString(L"JPEG Image");
                }
                else if (::lstrcmpiW(pExt, L".png") == 0)
                {
                    pv->bstrVal = ::SysAllocString(L"PNG Image");
                }
                else
                {
                    pv->bstrVal = ::SysAllocString(L"File");
                }
            }
            else
            {
                pv->bstrVal = ::SysAllocString(L"File");
            }
        }
        else
        {
            pv->bstrVal = nullptr;
            hr = E_NOTIMPL;
            goto End;
        }

        if (pv->bstrVal == nullptr)
        {
            hr = E_OUTOFMEMORY;
            goto End;
        }

        hr = S_OK;
        break;
    }
    }

End:

    if (pidlAbsolute)
    {
        ::ILFree(pidlAbsolute);
        pidlAbsolute = nullptr;
    }

    if (bstrPath)
    {
        ::SysFreeString(bstrPath);
        bstrPath = nullptr;
    }

    if (pInterfaceProvider)
    {
        delete pInterfaceProvider;
        pInterfaceProvider = nullptr;
    }

    if (pBigDriveFileInfo)
    {
        pBigDriveFileInfo->Release();
        pBigDriveFileInfo = nullptr;
    }

    return hr;
}