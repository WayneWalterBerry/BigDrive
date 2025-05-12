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
        return GetApplications(m_pIDispatch, &m_ppApplications, m_dwSize);
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

    if (index >= m_dwSize || m_ppApplications == nullptr)
    {
        return E_BOUNDS; // Index out of range or array uninitialized
    }

    *ppApplication = m_ppApplications[index];
    return S_OK;
}

/// <summary>
/// Populates the specified COM+ collection by invoking the "Populate" method.
/// </summary>
/// <returns>HRESULT indicating success or failure of the operation.</returns>
HRESULT ApplicationCollection::Populate(LPDISPATCH pIDispatch)
{
    HRESULT hrReturn = S_OK;

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    DISPID dispidPopulate;
    const OLECHAR* populateMethod = L"Populate";

    // Get the DISPID for the "Populate" method
    LPOLESTR mutablePopulateMethod = const_cast<LPOLESTR>(populateMethod);
    hrReturn = pIDispatch->GetIDsOfNames(IID_NULL, &mutablePopulateMethod, 1, LOCALE_USER_DEFAULT, &dispidPopulate);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Failed to get DISPID for 'Populate'. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Invoke the "Populate" method
    hrReturn = pIDispatch->Invoke(dispidPopulate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Failed to invoke 'Populate'. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

End:

    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Operation failed. HRESULT: 0x%08X", hrReturn);
    }

    return hrReturn;
}

HRESULT ApplicationCollection::GetApplications(LPDISPATCH pIDispatch, Application*** pppApplications, DWORD& dwSize)
{
    HRESULT hrReturn = S_OK;

    IDispatch* pCollection = nullptr;

    dwSize = 0;

    Dispatch dispatch(pIDispatch);

    // Populate the Applications collection
    hrReturn = Populate(pIDispatch);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Retrieve the count of applications
    LONG lCount;
    hrReturn = dispatch.GetLongProperty(L"Count", lCount);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to retrieve application count. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Allocate memory for the IDispatch pointers
    *pppApplications = (Application**)::CoTaskMemAlloc(sizeof(Application*) * lCount);
    if (*pppApplications == nullptr)
    {
        hrReturn = E_OUTOFMEMORY;
        s_eventLogger.WriteError(L"GetApplications: Memory allocation for application pointers failed.");
        goto End;
    }

    // Retrieve each application and store its IDispatch pointer
    for (long i = 0; i < lCount; i++)
    {
        DISPID dispidItem;
        const OLECHAR* szMethodName = L"Item";

        // Get the DISPID for the Item property
        hrReturn = pIDispatch->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&szMethodName), 1, LOCALE_USER_DEFAULT, &dispidItem);
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
        hrReturn = pIDispatch->Invoke(dispidItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &indexParams, &vtCollection, nullptr, nullptr);
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
        (*pppApplications)[i] = new Application(vtCollection.pdispVal);
        dwSize++;
    }

    goto End;

CleanupArray:

    // Cleanup allocated memory and release IDispatch pointers on failure
    if (*pppApplications != nullptr)
    {
        for (DWORD j = 0; j < dwSize; j++)
        {
            if ((*pppApplications)[j] != nullptr)
            {
                delete (*pppApplications)[j];
            }
        }
        ::CoTaskMemFree(*pppApplications);
        *pppApplications = nullptr;
        dwSize = 0;
    }

End:
    if (pCollection)
    {
        pCollection->Release();
        pCollection = nullptr;
    }


    return hrReturn;
}