// <copyright file="Application.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comadmin.h>
#include <oaidl.h>

// Header
#include "Application.h"

HRESULT Application::GetName(BSTR& bstrString)
{
    return GetStringProperty(L"Name", bstrString);
}

HRESULT Application::GetComponentsCLSIDs(CLSID** ppCLSIDs, long& lSize)
{
    return S_OK;
}
