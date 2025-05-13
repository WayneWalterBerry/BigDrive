// <copyright file="ComponentCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <comadmin.h>
#include <oaidl.h>

// Header
#include "ComponentCollection.h"

// Local
#include "Component.h"

HRESULT ComponentCollection::GetName(BSTR& bstrString)
{
    return GetStringProperty(L"Name", bstrString);
}

HRESULT ComponentCollection::GetComponents(Component*** pppComponents, long& lSize)
{
    HRESULT hrReturn = S_OK;

    lSize = 0;

    if (pppComponents == nullptr)
    {
        return E_POINTER;
    }

    // Populate the Applications collection
    hrReturn = Populate();
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetComponentsCLSIDs: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Retrieve the count of applications
    hrReturn = GetCount(lSize);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetComponentsCLSIDs: Failed to get count of applications. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Allocate memory for the Application Instances
    *pppComponents = (Component**)::CoTaskMemAlloc(sizeof(Component*) * lSize);
    if (*pppComponents == nullptr)
    {
        hrReturn = E_OUTOFMEMORY;
        s_eventLogger.WriteError(L"GetComponentsCLSIDs: Memory allocation for application pointers failed.");
        goto End;
    }

End:

    return hrReturn;
}
