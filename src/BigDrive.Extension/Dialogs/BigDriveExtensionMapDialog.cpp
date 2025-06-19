// <copyright file="BigDriveExtensionMapDialog.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveExtensionMapDialog.h"
#include "..\resource.h"
#include "..\Logging\BigDriveTraceLogger.h"
#include "..\dllmain.h"
#include "..\..\BigDrive.Client\BigDriveService.h"

#include <guiddef.h>
#include <wchar.h>

HRESULT BigDriveExtensionMapDialog::ShowDialog(HINSTANCE hInstance, HWND hParent)
{
    HRESULT hr = S_OK;

    BigDriveTraceLogger::LogEnter(__FUNCTION__);

    if (::FindResourceW(g_hInstance, MAKEINTRESOURCEW(IDD_BIGDRIVE_MAP_DIALOG), RT_DIALOG) == nullptr)
    {
		BigDriveTraceLogger::LogError(__FUNCTION__, L"Failed to find dialog resource IDD_BIGDRIVE_MAP_DIALOG.");
    }

    m_hDlg = ::CreateDialogParamW(
        g_hInstance,
        MAKEINTRESOURCEW(IDD_BIGDRIVE_MAP_DIALOG),
        hParent,
        BigDriveExtensionMapDialog::DialogProc,
        reinterpret_cast<LPARAM>(this));

    if (m_hDlg == nullptr)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to create dialog window.", hr);
        goto End;
    }

    ::ShowWindow(m_hDlg, SW_SHOW);

End:

    BigDriveTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}

INT_PTR CALLBACK BigDriveExtensionMapDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = S_OK;
    HWND hParent;
    RECT rcParent = { 0 };
    RECT rcDlg = { 0 };
    int x = 0, y = 0;

    BigDriveTraceLogger::LogEnter(__FUNCTION__);

    if (message == WM_INITDIALOG)
    {
        ::SetWindowLongPtrW(hDlg, GWLP_USERDATA, lParam);
    }

    BigDriveExtensionMapDialog* pThis = reinterpret_cast<BigDriveExtensionMapDialog*>(::GetWindowLongPtrW(hDlg, GWLP_USERDATA));

    switch (message)
    {
    case WM_INITDIALOG:

        // Center the dialog over its parent
        hParent = ::GetParent(hDlg);

        if (hParent != nullptr && ::GetWindowRect(hParent, &rcParent) && ::GetWindowRect(hDlg, &rcDlg))
        {
            int dlgWidth = rcDlg.right - rcDlg.left;
            int dlgHeight = rcDlg.bottom - rcDlg.top;
            int parentWidth = rcParent.right - rcParent.left;
            int parentHeight = rcParent.bottom - rcParent.top;

            x = rcParent.left + (parentWidth - dlgWidth) / 2;
            y = rcParent.top + (parentHeight - dlgHeight) / 2;

            ::SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
        else if (::GetWindowRect(hDlg, &rcDlg))
        {
            // Center on screen if no parent
            int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
            int dlgWidth = rcDlg.right - rcDlg.left;
            int dlgHeight = rcDlg.bottom - rcDlg.top;

            x = (screenWidth - dlgWidth) / 2;
            y = (screenHeight - dlgHeight) / 2;

            ::SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
        break;

    case WM_COMMAND:

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            BigDriveService service;

            hr = service.CreateSampleProviderDrive();
            if (FAILED(hr))
            {
                BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to create sample provider drive.", hr);
			}

            ::DestroyWindow(hDlg);
            return (INT_PTR)TRUE;
        }

        break;
    case WM_CLOSE:

        ::DestroyWindow(hDlg);
        return (INT_PTR)TRUE;

    case WM_NCDESTROY:

        // Self-delete after window is destroyed
        if (pThis != nullptr)
        {
            delete pThis;
        }

        break;
    }

    BigDriveTraceLogger::LogExit(__FUNCTION__, hr);

    return (INT_PTR)FALSE;
}


