// <copyright file="BigDriveExtensionMapDialog.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

class BigDriveExtensionMapDialog
{
public:

    /// <summary>
    /// Shows the modeless dialog.
    /// </summary>
    /// <param name="hInstance">Application instance handle.</param>
    /// <param name="hParent">Parent window handle.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    HRESULT ShowDialog(HINSTANCE hInstance, HWND hParent);

private:

    HWND m_hDlg;

    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};