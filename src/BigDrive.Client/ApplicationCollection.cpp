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

    *ppApplication = m_ppApplications[index];
    return S_OK;
}

HRESULT ApplicationCollection::GetApplications(Application*** pppApplications, LONG& lSize)
{
    HRESULT hrReturn = S_OK;

    lSize = 0;

    // Populate the Applications collection
    hrReturn = Populate();
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Retrieve the count of applications
    hrReturn = GetCount(lSize);
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
        DISPID dispidItem;
        const OLECHAR* szMethodName = L"Item";

        // Get the DISPID for the Item property
        hrReturn = GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&szMethodName), 1, LOCALE_USER_DEFAULT, &dispidItem);
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to get DISPID for Item property. HRESULT: 0x%08X", hrReturn);
            goto CleanupArray;
        }

        VARIANT vtItem;
        VariantInit(&vtItem);
        DISPPARAMS indexParams = { &vtItem, nullptr, 1, 0 };
        vtItem.vt = VT_I4;
        vtItem.lVal = i;

        VARIANT vtCollection;
        VariantInit(&vtCollection);

        // Invoke the Item property to retrieve the application
        hrReturn = m_pIDispatch->Invoke(dispidItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &indexParams, &vtCollection, nullptr, nullptr);
        if (FAILED(hrReturn))
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to invoke Item property for index %ld. HRESULT: 0x%08X", i, hrReturn);
            goto CleanupArray;
        }

        if (vtCollection.vt != VT_DISPATCH)
        {
            s_eventLogger.WriteErrorFormmated(L"GetApplications: Item at index %ld is not a valid IDispatch pointer. HRESULT: 0x%08X", i, hrReturn);
            goto CleanupArray;
        }

        // Store the IDispatch pointer in the array
        (*pppApplications)[i] = new Application(m_pCOMAdminCatalog, vtCollection.pdispVal);

        ::VariantClear(&vtCollection);
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