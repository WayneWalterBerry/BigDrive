// <copyright file="BigDriveExtensionMapDialog.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveExtensionMapDialog.h"
#include "..\resource.h"

HRESULT BigDriveExtensionMapDialog::ShowDialog(HINSTANCE hInstance, HWND hParent)
{
    HRESULT hr = S_OK;

    m_hDlg = ::CreateDialogParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_BIGDRIVE_MAP_DIALOG),
        hParent,
        BigDriveExtensionMapDialog::DialogProc,
        reinterpret_cast<LPARAM>(this));

    if (m_hDlg == nullptr)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto End;
    }

    ::ShowWindow(m_hDlg, SW_SHOW);

End:

    return hr;
}

INT_PTR CALLBACK BigDriveExtensionMapDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG)
    {
        ::SetWindowLongPtrW(hDlg, GWLP_USERDATA, lParam);
    }

    BigDriveExtensionMapDialog* pThis = reinterpret_cast<BigDriveExtensionMapDialog*>(::GetWindowLongPtrW(hDlg, GWLP_USERDATA));

    switch (message)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
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
    return (INT_PTR)FALSE;
}