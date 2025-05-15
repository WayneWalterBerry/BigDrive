// <copyright file="CatalogObject.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "CatalogObject.h"

HRESULT CatalogObject::GetName(BSTR& bstrName)
{
    return GetStringProperty(L"Name", bstrName);
}

HRESULT CatalogObject::GetDescription(BSTR& bstrDescription)
{
    return GetValue(L"Description", bstrDescription);
}

/// <summary>
/// Gets The Application Key
/// </summary>
/// <param name="bstrKey">Application Key</param>
HRESULT CatalogObject::GetKey(BSTR& bstrKey)
{
    return GetStringProperty(L"Key", bstrKey);
}