// <copyright file="COM.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

class COM
{
public:

    static HRESULT GetProperty(IDispatch *pIDispatch, LPCWSTR szName, VARIANT* pValue);
    static HRESULT GetStringProperty(IDispatch* pIDispatch, LPCWSTR szName, BSTR& bstrString);
    static HRESULT GetLongProperty(IDispatch* pIDispatch, LPCWSTR szName, LONG& value);
};