// <copyright file="CatalogCollection.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "CatalogCollection.h"

// Initialize the static EventLogger instance
EventLogger CatalogCollection::s_eventLogger(L"BigDrive.Client");

HRESULT CatalogCollection::Populate(LPDISPATCH pIDispatch)
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