// <copyright file="ApplicationManager.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <comdef.h>
#include <iostream>

// Header
#include "ApplicationManager.h"

// Local
#include "Dispatch.h"

// Initialize the static EventLogger instance
EventLogger ApplicationManager::s_eventLogger(L"BigDrive.Client");

HRESULT ApplicationManager::StartApplication(CLSID clsidProvider)
{
    HRESULT hrReturn = S_OK;
    CLSID clsidCOMAdminCatalog;
    IDispatch* pIDispatchCatalog = nullptr;
    DISPID dispid;
    LPOLESTR progID = nullptr;
    const OLECHAR* methodName = L"StartApplication";

    // Invoke StartApplication
    DISPPARAMS params = {};
    VARIANTARG varg = {};

    varg.vt = VT_BSTR;
    params.rgvarg = &varg;
    params.cArgs = 1;

    hrReturn = ::ProgIDFromCLSID(clsidProvider, &progID);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to retrieve ProgID from CLSID. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    varg.bstrVal = progID;

    // Create COMAdminCatalog instance
    hrReturn = ::CLSIDFromProgID(L"COMAdmin.COMAdminCatalog", &clsidCOMAdminCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to retrieve CLSID for COMAdminCatalog. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = ::CoCreateInstance(clsidCOMAdminCatalog, nullptr, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**)&pIDispatchCatalog);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to create COMAdminCatalog instance. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    // Prepare arguments for StartApplication
    hrReturn = pIDispatchCatalog->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&methodName), 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to get DISPID for StartApplication. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

    hrReturn = pIDispatchCatalog->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);
    if (FAILED(hrReturn))
    {
        s_eventLogger.WriteErrorFormmated(L"Failed to invoke StartApplication. HRESULT: 0x%08X", hrReturn);
        goto End;
    }

End:

    // Cleanup
    if (progID != nullptr)
    {
        ::CoTaskMemFree(progID);
        progID = nullptr;
    }

    if (pIDispatchCatalog != nullptr)
    {
        pIDispatchCatalog->Release();
        pIDispatchCatalog = nullptr;
    }

    return hrReturn;
}





