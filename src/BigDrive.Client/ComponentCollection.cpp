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
        Component** temp = nullptr;
        LONG tempSize = 0;

        HRESULT hr = GetComponents(&temp, tempSize);

        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"Initialize: Failed to get components. HRESULT: 0x%08X", hr);
            return hr;
        }

        // Only set m_ppComponents if it is still nullptr
        if (InterlockedCompareExchangePointer((PVOID*)&m_ppComponents, temp, nullptr) != nullptr)
        {
            // Another thread already initialized, so free our temp
            for (LONG i = 0; i < tempSize; ++i)
            {
                if (temp[i])
                {
                    delete temp[i];
                }
            }
            ::CoTaskMemFree(temp);
        }
        else
        {
            m_lSize = tempSize;
        }
    }
    return S_OK;
}

HRESULT ComponentCollection::GetName(BSTR& bstrString)
{
    return GetStringProperty(L"Name", bstrString);
}

/// <inheritdoc />
HRESULT ComponentCollection::GetItem(size_t index, Component** ppComponent)
{
    if (ppComponent == nullptr)
    {
        return E_POINTER;
    }

    HRESULT hr = Initialize();
    if (FAILED(hr) || (m_ppComponents == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetItem: Failed to initialize ComponentCollection. HRESULT: 0x%08X", hr);
        return hr;
    }

    if (index >= m_lSize)
    {
        return E_BOUNDS; // Index out of range or array uninitialized
    }

    return m_ppComponents[index]->Clone(ppComponent);
}

/// <inheritdoc />
HRESULT ComponentCollection::GetComponents(Component*** pppComponents, LONG& lSize)
{
    HRESULT hr = S_OK;

    lSize = 0;

    ICatalogCollection* pICatalogCollection = nullptr;
    hr = GetICatalogCollection(&pICatalogCollection);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get ICatalogCollection. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pICatalogCollection->Populate();
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate component collection. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pICatalogCollection->get_Count(&lSize);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get count of components. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Allocate memory for the Application Instances
    *pppComponents = (Component**)::CoTaskMemAlloc(sizeof(Component*) * lSize);
    if (*pppComponents == nullptr)
    {
        hr = E_OUTOFMEMORY;
        s_eventLogger.WriteError(L"GetApplications: Memory allocation for component pointers failed.");
        goto End;
    }

    // Retrieve each application and store its IDispatch pointer
    for (long i = 0; i < lSize; i++)
    {
        IDispatch* pDispatch = nullptr;

        hr = pICatalogCollection->get_Item(i, &pDispatch);
        if (FAILED(hr) || pDispatch == nullptr)
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get component at index %d. HRESULT: 0x%08X", i, hr);
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

    return hr;
}
