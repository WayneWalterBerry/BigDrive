// <copyright file="ComponentCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <oaidl.h>

// Header
#include "ComponentCollection.h"

// Local
#include "Component.h"

HRESULT ComponentCollection::Initialize()
{
    if (m_ppComponents == nullptr)
    {
        return GetComponents(&m_ppComponents, m_lSize);
    }

    return S_OK;
}

HRESULT ComponentCollection::GetName(BSTR& bstrString)
{
    return GetStringProperty(L"Name", bstrString);
}

/// <inheritdoc />
HRESULT ComponentCollection::GetItem(size_t index, Component** ppComponent) const
{
    if (ppComponent == nullptr)
    {
        return E_POINTER;
    }

    if (index >= m_lSize || ppComponent == nullptr)
    {
        return E_BOUNDS; // Index out of range or array uninitialized
    }

    return m_ppComponents[index]->Clone(ppComponent);
}

/// <inheritdoc />
HRESULT ComponentCollection::GetComponents(Component*** pppComponents, LONG& lSize)
{
    HRESULT hrReturn = S_OK;

    lSize = 0;

    ICatalogCollection* pICatalogCollection = nullptr;
    hrReturn = GetICatalogCollection(&pICatalogCollection);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get ICatalogCollection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = pICatalogCollection->Populate();
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate component collection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = pICatalogCollection->get_Count(&lSize);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get count of components. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Allocate memory for the Application Instances
    *pppComponents = (Component**)::CoTaskMemAlloc(sizeof(Component*) * lSize);
    if (*pppComponents == nullptr)
    {
        hrReturn = E_OUTOFMEMORY;
        s_eventLogger.WriteError(L"GetApplications: Memory allocation for component pointers failed.");
        goto End;
    }

    // Retrieve each application and store its IDispatch pointer
    for (long i = 0; i < lSize; i++)
    {
        IDispatch* pDispatch = nullptr;

        hrReturn = pICatalogCollection->get_Item(i, &pDispatch);
        if (FAILED(hrReturn) || pDispatch == nullptr)
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get component at index %d. HRESULT: 0x%08X", i, hrReturn);
            goto CleanupArray;
        }

        // Store the IDispatch pointer in the array
        (*pppComponents)[i] = new Component(pDispatch);
    }

    goto End;

CleanupArray:

    // Cleanup allocated memory and release IDispatch pointers on failure
    if (*pppComponents != nullptr)
    {
        for (LONG j = 0; j < lSize; j++)
        {
            if ((*pppComponents)[j] != nullptr)
            {
                delete (*pppComponents)[j];
            }
        }
        ::CoTaskMemFree(*pppComponents);
        *pppComponents = nullptr;
        lSize = 0;
    }

End:

    return hrReturn;
}
