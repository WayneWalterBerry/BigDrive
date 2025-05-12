// <copyright file="Dispatch.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

class Dispatch
{
protected:

    LPDISPATCH m_pIDispatch;

public:

    Dispatch(LPDISPATCH pIDispatch)
        : m_pIDispatch(pIDispatch)
    {
        if (m_pIDispatch == nullptr)
        {
            throw E_POINTER;
        }

        if (m_pIDispatch)
        {
            m_pIDispatch->AddRef();
        }
    }

    ~Dispatch()
    {
        if (m_pIDispatch)
        {
            m_pIDispatch->Release();
            m_pIDispatch = nullptr;
        }
    }

    HRESULT GetProperty(LPCWSTR szName, VARIANT* pValue);
    HRESULT GetStringProperty(LPCWSTR szName, BSTR& bstrString);
    HRESULT GetLongProperty(LPCWSTR szName, LONG& value);
};