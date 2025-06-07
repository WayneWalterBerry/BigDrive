// <copyright file="BigDriveDropTarget.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <shlobj.h>

#include "BigDriveShellFolder.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

class BigDriveDropTarget : public IDropTarget
{
private:

    LONG m_cRef;
    BigDriveShellFolder* m_pFolder;
    BOOL m_fAllowDrop;
    DWORD m_dwEffect;

    /// <summary>
    /// Logger that captures trace information for the shell folder.
    /// </summary>
    BigDriveShellFolderTraceLogger m_traceLogger;

public:
    BigDriveDropTarget(BigDriveShellFolder* pFolder);
    ~BigDriveDropTarget();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IDropTarget
    STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

private:
    BOOL IsFormatSupported(IDataObject* pDataObj);
    HRESULT ProcessDrop(IDataObject* pDataObj);
};