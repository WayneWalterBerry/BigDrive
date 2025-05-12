// <copyright file="Application.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "COM.h"

class Application : COM
{
private:

    LPDISPATCH m_pDispatch;

public:

    Application(LPDISPATCH pDispatch)
        : m_pDispatch(pDispatch)
    {
        if (m_pDispatch)
        {
            m_pDispatch->AddRef();
        }
    }

    ~Application()
    {
        if (m_pDispatch)
        {
            m_pDispatch->Release();
            m_pDispatch = nullptr;
        }
    }

    HRESULT GetAppId(BSTR& bstrString);
};
