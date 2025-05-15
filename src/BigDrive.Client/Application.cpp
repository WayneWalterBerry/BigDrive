// <copyright file="Application.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <oaidl.h>

// Header
#include "Application.h"

// Local
#include "Component.h"

/// <inheritdoc />
HRESULT Application::GetId(BSTR& bstrId)
{
    return GetValue(L"ID", bstrId);
}