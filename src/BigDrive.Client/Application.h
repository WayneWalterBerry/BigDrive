// <copyright file="Application.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// System
#include <comdef.h>

// Local
#include "Dispatch.h"

class Application : Dispatch
{

public:

    Application(LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
    }

    ~Application()
    {
    }

    HRESULT GetAppId(BSTR& bstrString);
};
