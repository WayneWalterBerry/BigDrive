// <copyright file="ApplicationCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "ApplicationCollection.h"
#include "ApplicationManager.h"

// Initialize the static EventLogger instance
EventLogger ApplicationCollection::s_eventLogger(L"BigDrive.Client");

/// <summary>
/// Populates the specified COM+ collection by invoking the "Populate" method.
/// </summary>
/// <returns>HRESULT indicating success or failure of the operation.</returns>
HRESULT ApplicationCollection::Populate()
{
    HRESULT hrReturn = S_OK;

    DISPPARAMS params = { nullptr, nullptr, 0, 0 };
    DISPID dispidPopulate;
    const OLECHAR* populateMethod = L"Populate";

    // Get the DISPID for the "Populate" method
    LPOLESTR mutablePopulateMethod = const_cast<LPOLESTR>(populateMethod);
    hrReturn = m_pDispatch->GetIDsOfNames(IID_NULL, &mutablePopulateMethod, 1, LOCALE_USER_DEFAULT, &dispidPopulate);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Populate: Failed to get DISPID for 'Populate'. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Invoke the "Populate" method
    hrReturn = m_pDispatch->Invoke(dispidPopulate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);
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

HRESULT ApplicationCollection::GetApplications(Application*** pppApplications, DWORD &dwSize)
{
    HRESULT hrReturn = S_OK;

    IDispatch* pCollection = nullptr;

    dwSize = 0;

    // Populate the Applications collection
    hrReturn = Populate();
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplications: Failed to populate Applications collection. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Retrieve the count of applications
    LONG lCount;
    hrReturn = COM::GetLongProperty(m_pDispatch, L"Count", lCount);
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
        hrReturn = m_pDispatch->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&szMethodName), 1, LOCALE_USER_DEFAULT, &dispidItem);
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
        hrReturn = m_pDispatch->Invoke(dispidItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &indexParams, &vtCollection, nullptr, nullptr);
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