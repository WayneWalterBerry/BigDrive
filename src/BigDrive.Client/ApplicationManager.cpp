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
#include "COMAdminCatalog.h"
#include "Interfaces/IBigDriveRegistration.h"

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

HRESULT ApplicationManager::RegisterApplications()
{
    HRESULT hr = S_OK;
    COMAdminCatalog* pCOMAdminCatalog = nullptr;
    ApplicationCollection* pApplicationCollection = nullptr;
    Application* pApplication = nullptr;
    ComponentCollection* pComponentCollection = nullptr;
    Component* pComponent = nullptr;
    IBigDriveRegistration* pBigDriveRegistration = nullptr;

    BSTR bstrApplicationName = nullptr;
    BSTR bstrComponentName = nullptr;

    hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Create() failed. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetApplicationsCollection() failed. HRESULT: 0x%08X", hr);
        goto End;
    }

    hr = pApplicationCollection->Initialize();
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"Initialize() failed. HRESULT: 0x%08X", hr);
        goto End;
    }

    LONG lApplicationCount;
    hr = pApplicationCollection->GetCount(lApplicationCount);
    if (FAILED(hr))
    {
        s_eventLogger.WriteErrorFormmated(L"GetCount() failed. HRESULT: 0x%08X", hr);
        goto End;
    }

    for (LONG a = 0; a < lApplicationCount; a++)
    {
        Application* pApplication = nullptr;
        hr = pApplicationCollection->GetItem(a, &pApplication);
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"GetItem() failed. HRESULT: 0x%08X", hr);
            goto End;
        }

        hr = pApplication->GetName(bstrApplicationName);
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"GetName() failed. HRESULT: 0x%08X", hr);
            goto End;
        }

        hr = pCOMAdminCatalog->GetComponentCollection(pApplication, &pComponentCollection);
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"GetComponentCollection() failed. HRESULT: 0x%08X", hr);
            goto End;
        }

        hr = pComponentCollection->Initialize();
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"Initialize() failed. HRESULT: 0x%08X", hr);
            goto End;
        }

        LONG lComponentCount;
        hr = pComponentCollection->GetCount(lComponentCount);
        if (FAILED(hr))
        {
            s_eventLogger.WriteErrorFormmated(L"GetCount() failed. HRESULT: 0x%08X", hr);
            goto End;
        }

        for (LONG c = 0; c < lComponentCount; c++)
        {
            hr = pComponentCollection->GetItem(c, &pComponent);
            if (FAILED(hr))
            {
                s_eventLogger.WriteErrorFormmated(L"GetItem() failed. HRESULT: 0x%08X", hr);
                goto End;
            }

            hr = pComponent->GetName(bstrComponentName);
            if (FAILED(hr))
            {
                s_eventLogger.WriteErrorFormmated(L"GetName() failed. HRESULT: 0x%08X", hr);
                goto End;
            }

            hr = pComponent->QueryInterface(IID_IBigDriveRegistration, (void**)&pBigDriveRegistration);
            switch (hr)
            {
            case S_OK:

                // Call Register() on the IBigDriveRegistration interface
                hr = pBigDriveRegistration->Register();
                if (FAILED(hr))
                {
                    s_eventLogger.WriteErrorFormmated(L"Register() failed. HRESULT: 0x%08X", hr);
                    goto End;
                }
                break;
            case E_NOINTERFACE:

                // Not Every Component Implements IBigDriveRegistration.
                hr = S_OK;
                continue;

            default:
                // Handle other cases if needed
                s_eventLogger.WriteErrorFormmated(L"QueryInterface() failed. HRESULT: 0x%08X", hr);
                break;
            }

            if (bstrComponentName)
            {
                ::SysFreeString(bstrComponentName);
                bstrComponentName = nullptr;
            }

            if (pComponent != nullptr)
            {
                pComponent->Release();
                pComponent = nullptr;
            }
        }

        if (bstrApplicationName)
        {
            ::SysFreeString(bstrApplicationName);
            bstrApplicationName = nullptr;
        }

        if (pComponentCollection != nullptr)
        {
            pComponentCollection->Release();
            pComponentCollection = nullptr;
        }

        if (pApplication != nullptr)
        {
            pApplication->Release();
            pApplication = nullptr;
        }
    }

End:

    // Clean up IBigDriveRegistration
    if (pBigDriveRegistration != nullptr)
    {
        pBigDriveRegistration->Release();
        pBigDriveRegistration = nullptr;
    }

    // Clean up Component
    if (pComponent != nullptr)
    {
        delete pComponent;
        pComponent = nullptr;
    }

    // Clean up ComponentCollection
    if (pComponentCollection != nullptr)
    {
        delete pComponentCollection;
        pComponentCollection = nullptr;
    }

    // Clean up Application
    if (pApplication != nullptr)
    {
        delete pApplication;
        pApplication = nullptr;
    }

    // Clean up ApplicationCollection
    if (pApplicationCollection != nullptr)
    {
        delete pApplicationCollection;
        pApplicationCollection = nullptr;
    }

    // Clean up COMAdminCatalog
    if (pCOMAdminCatalog != nullptr)
    {
        delete pCOMAdminCatalog;
        pCOMAdminCatalog = nullptr;
    }

    return hr;

}






