// <copyright file="ApplicationCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "ApplicationCollection.h"
#include "ApplicationManager.h"

/// <inheritdoc />
ULONG ApplicationCollection::Release()
{
    // Call delete on the object, not release.
    return E_FAIL;
}

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
HRESULT ApplicationCollection::QueryApplicationByName(LPCWSTR szName, Application** ppApplication)
{
    HRESULT hr = S_OK;

    if (szName == nullptr || ppApplication == nullptr)
    {
        s_eventLogger.WriteError(L"QueryApplicationByName: Invalid argument(s).");
        return E_POINTER;
    }

    hr = Initialize();
    if (FAILED(hr) || (m_ppApplications == nullptr))
    {
        s_eventLogger.WriteErrorFormmated(L"QueryApplicationByName: Failed to initialize ApplicationCollection. HRESULT: 0x%08X", hr);
        return hr;
    }

    *ppApplication = nullptr;

    for (LONG i = 0; i < m_lSize; ++i)
    {
        BSTR bstrCurrentName = nullptr;
        HRESULT hrName = m_ppApplications[i]->GetName(bstrCurrentName);
        if (FAILED(hrName))
        {
            s_eventLogger.WriteErrorFormmated(L"QueryApplicationByName: GetName failed for index %d. HRESULT: 0x%08X", i, hrName);
            continue;
        }

        if (bstrCurrentName && wcscmp(bstrCurrentName, szName) == 0)
        {
            HRESULT hrClone = m_ppApplications[i]->Clone(ppApplication);
            ::SysFreeString(bstrCurrentName);
            if (FAILED(hrClone))
            {
                s_eventLogger.WriteErrorFormmated(L"QueryApplicationByName: Clone failed for index %d. HRESULT: 0x%08X", i, hrClone);
                return hrClone;
            }
            return S_OK;
        }

        if (bstrCurrentName)
        {
            ::SysFreeString(bstrCurrentName);
        }
    }

    s_eventLogger.WriteErrorFormmated(L"QueryApplicationByName: No application found with name '%s'.", szName);
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}


/// <inheritdoc />
HRESULT ApplicationCollection::GetApplications(Application*** pppApplications, LONG& lSize)
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
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate Applications collection. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pICatalogCollection->get_Count(&lSize);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get count of applications. HRESULT: 0x%08X", hr);
        goto End;
    }

    // Allocate memory for the Application Instances
    *pppApplications = (Application**)::CoTaskMemAlloc(sizeof(Application*) * lSize);
    if (*pppApplications == nullptr)
    {
        hr = E_OUTOFMEMORY;
        s_eventLogger.WriteError(L"GetApplications: Memory allocation for application pointers failed.");
        goto End;
    }

    // Retrieve each application and store its IDispatch pointer
    for (long i = 0; i < lSize; i++)
    {
        IDispatch* pDispatch = nullptr;

        hr = pICatalogCollection->get_Item(i, &pDispatch);
        if (FAILED(hr) || pDispatch == nullptr)
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get application at index %d. HRESULT: 0x%08X", i, hr);
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

    return hr;
}


