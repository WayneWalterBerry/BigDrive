// <copyright file="ApplicationCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "ApplicationCollection.h"
#include "ApplicationManager.h"

HRESULT ApplicationCollection::Initialize()
{
    if (m_ppApplications == nullptr)
    {
        return GetApplications(&m_ppApplications, m_lSize);
    }

    return S_OK;
}

/// <summary>
/// Provides access to an Application object at the specified index.
/// </summary>
/// <param name="index">The zero-based index of the Application to retrieve.</param>
/// <param name="ppApplication">Pointer to receive the Application object at the specified index.</param>
/// <returns>HRESULT indicating success or failure of the operation.</returns>
HRESULT ApplicationCollection::GetItem(size_t index, Application** ppApplication) const
{
    if (ppApplication == nullptr)
    {
        return E_POINTER;
    }

    if (index >= m_lSize || m_ppApplications == nullptr)
    {
        return E_BOUNDS; // Index out of range or array uninitialized
    }

    return m_ppApplications[index]->Clone(ppApplication);
}

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