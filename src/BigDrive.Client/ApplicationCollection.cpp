// <copyright file="ApplicationCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "ApplicationCollection.h"
#include "ApplicationManager.h"

/// <inheritdoc />
HRESULT ApplicationCollection::Initialize()
{
    if (m_ppApplications == NULL)
    {
        Application** temp = NULL;
        LONG tempSize = 0;

        HRESULT hr = GetApplications(&temp, tempSize);

        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"Initialize: Failed to get applications. HRESULT: 0x%08X", hr);
            return hr;
        }

        // Only set m_ppApplications if it is still NULL
        if (InterlockedCompareExchangePointer((PVOID*)&m_ppApplications, temp, NULL) != NULL)
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

/// <inheritdoc />
HRESULT ApplicationCollection::GetItem(size_t index, Application** ppApplication)
{
    if (ppApplication == nullptr)
    {
        return E_POINTER;
    }

    HRESULT hr = Initialize();
    if (FAILED(hr) || (m_ppApplications==nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetItem: Failed to initialize ApplicationCollection. HRESULT: 0x%08X", hr);
        return hr;
    }

    if (index >= m_lSize)
    {
        return E_BOUNDS; // Index out of range or array uninitialized
    }

    return m_ppApplications[index]->Clone(ppApplication);
}

/// <inheritdoc />
HRESULT ApplicationCollection::GetApplications(Application*** pppApplications, LONG& lSize)
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
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = pICatalogCollection->get_Count(&lSize);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get count of applications. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Allocate memory for the Application Instances
    *pppApplications = (Application**)::CoTaskMemAlloc(sizeof(Application*) * lSize);
    if (*pppApplications == nullptr)
    {
        hrReturn = E_OUTOFMEMORY;
        s_eventLogger.WriteError(L"GetApplications: Memory allocation for application pointers failed.");
        goto End;
    }

    // Retrieve each application and store its IDispatch pointer
    for (long i = 0; i < lSize; i++)
    {
        IDispatch* pDispatch = nullptr;

        hrReturn = pICatalogCollection->get_Item(i, &pDispatch);
        if (FAILED(hrReturn) || pDispatch == nullptr)
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get application at index %d. HRESULT: 0x%08X", i, hrReturn);
            goto CleanupArray;
        }

        // Store the IDispatch pointer in the array
        (*pppApplications)[i] = new Application(pDispatch);
    }

    goto End;

CleanupArray:

    // Cleanup allocated memory and release IDispatch pointers on failure
    if (*pppApplications != nullptr)
    {
        for (LONG j = 0; j < lSize; j++)
        {
            if ((*pppApplications)[j] != nullptr)
            {
                delete (*pppApplications)[j];
            }
        }
        ::CoTaskMemFree(*pppApplications);
        *pppApplications = nullptr;
        lSize = 0;
    }

End:

    return hrReturn;
}