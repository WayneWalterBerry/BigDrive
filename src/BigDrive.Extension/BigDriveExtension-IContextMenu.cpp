// <copyright file="BigDriveExtension-IContextMenu.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveExtension.h"
#include "resource.h"
#include "Dialogs\BigDriveExtensionMapDialog.h"
#include "Logging\BigDriveTraceLogger.h"

#include <strsafe.h>

STDMETHODIMP BigDriveExtension::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT uFlags)
{
    HRESULT hr = S_OK;

    if (uFlags & CMF_DEFAULTONLY)
    {
        goto End;
    }

    hr = AddCustomMenuItem(hMenu, indexMenu, idCmdFirst);

End:

    return hr;
}

HRESULT BigDriveExtension::AddCustomMenuItem(HMENU& hMenu, UINT& indexMenu, UINT& idCmdFirst)
{
    HRESULT hr = S_OK;

    if (!::InsertMenuW(
        hMenu,
        indexMenu,
        MF_BYPOSITION,
        idCmdFirst,
        L"Map BigDrive..."))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto End;
    }

    // Only one custom item, so return S_OK.
    hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);

End:

    return hr;
}

STDMETHODIMP BigDriveExtension::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = S_OK;

    if (HIWORD(pici->lpVerb) != 0)
    {
        hr = E_FAIL;
        goto End;
    }

    if (LOWORD(pici->lpVerb) == 0)
    {
        // Create and show the modeless dialog using the dialog class
        BigDriveExtensionMapDialog* pDialog = new BigDriveExtensionMapDialog();
        if (pDialog == nullptr)
        {
			BigDriveTraceLogger::LogError(__FUNCTION__, L"Failed to allocate memory for BigDriveExtensionMapDialog.");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        hr = pDialog->ShowDialog(::GetModuleHandleW(nullptr), nullptr);
        if (FAILED(hr))
        {
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to show BigDriveExtensionMapDialog. HRESULT: 0x%08X", hr);
            delete pDialog;
            goto End;
        }
    }

End:

    return hr;
}

STDMETHODIMP BigDriveExtension::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* /*pReserved*/, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = S_OK;

    if (idCmd == 0 && uType == GCS_HELPTEXTA)
    {
        ::StringCchCopyA(pszName, cchMax, "Performs the BigDrive custom action.");
    }
    else if (idCmd == 0 && uType == GCS_HELPTEXTW)
    {
        ::StringCchCopyW(reinterpret_cast<LPWSTR>(pszName), cchMax, L"Performs the BigDrive custom action.");
    }

    return hr;
}