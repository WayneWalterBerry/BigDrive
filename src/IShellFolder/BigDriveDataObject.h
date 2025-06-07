// <copyright file="BigDriveDataObject.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <shlobj.h>
#include "BigDriveShellFolder.h"

class BigDriveDataObject : public IDataObject
{
private:
    LONG m_cRef;
    BigDriveShellFolder* m_pFolder;
    UINT m_cidl;
    PCUITEMID_CHILD* m_apidl;

public:

    BigDriveDataObject(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl);
    ~BigDriveDataObject();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IDataObject
    STDMETHODIMP GetData(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    STDMETHODIMP GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    STDMETHODIMP QueryGetData(FORMATETC* pformatetc);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut);
    STDMETHODIMP SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
    STDMETHODIMP DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

private:

    HRESULT CreateShellIDList(STGMEDIUM* pmedium);
    HRESULT CreateFileDescriptor(STGMEDIUM* pmedium);
};