// <copyright file="Application.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "Application.h"

HRESULT Application::GetAppId(BSTR& bstrString)
{
    return COM::GetStringProperty(m_pDispatch, L"AppID", bstrString);
}