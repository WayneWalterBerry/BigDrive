// <copyright file="BigDriveShellFolder-IShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

class EmptyEnumIDList : public IEnumIDList
{
    LONG m_refCount;

public:

    EmptyEnumIDList() : m_refCount(1) {}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IEnumIDList)
        {
            *ppv = static_cast<IEnumIDList*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override 
    {
        return InterlockedIncrement(&m_refCount); 
    }

    ULONG __stdcall Release() override
    {
        LONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return ref;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IEnumIDList

    HRESULT __stdcall Next(ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched) override
    {
        if (pceltFetched) *pceltFetched = 0;
        if (rgelt && celt > 0) *rgelt = nullptr;
        return S_FALSE; // No items
    }

    HRESULT __stdcall Skip(ULONG) override
    { 
        return S_FALSE;
    }

    HRESULT __stdcall Reset() override
    { 
        return S_OK; }

    HRESULT __stdcall Clone(IEnumIDList** ppenum) override
    {
        if (!ppenum) return E_POINTER;
        *ppenum = new EmptyEnumIDList();
        return S_OK;
    }
};