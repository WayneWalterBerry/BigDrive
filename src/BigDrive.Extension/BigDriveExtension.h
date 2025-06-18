// <copyright file="BigDriveExtension.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "pch.h"

#include <windows.h>
#include <shlobj.h>

// {CBB26998-8B10-4599-8AB7-01AF65F3F68B}
extern "C" const CLSID CLSID_BigDriveExtension;

/// <summary>
/// Implements a context menu extension for "My PC".
/// </summary>
class BigDriveExtension : public IContextMenu, public IShellExtInit
{
public:

    /// <summary>
    /// Standard constructor.
    /// </summary>
    BigDriveExtension();

    /// <summary>
    /// Standard destructor.
    /// </summary>
    virtual ~BigDriveExtension();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IShellExtInit
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hKeyProgID) override;

    // IContextMenu
    STDMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) override;
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici) override;
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax) override;

private:

    LONG m_cRef;

    /// <summary>
    /// Adds the custom menu item.
    /// </summary>
    HRESULT AddCustomMenuItem(HMENU& hMenu, UINT& indexMenu, UINT& idCmdFirst);

};