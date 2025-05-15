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

/// <inheritdoc />
HRESULT ApplicationManager::RegisterApplications()
{
    HRESULT hr = S_OK;
    COMAdminCatalog* pCOMAdminCatalog = nullptr;
    ApplicationCollection* pApplicationCollection = nullptr;
    Application* pApplication = nullptr;
    ComponentCollection* pComponentCollection = nullptr;
    Component* pComponent = nullptr;
    IBigDriveRegistration* pBigDriveRegistration = nullptr;
    CLSID clsid;

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

            hr = pComponent->GetCLSID(clsid);
            if (FAILED(hr))
            {
                s_eventLogger.WriteErrorFormmated(L"GetCLSID() failed. HRESULT: 0x%08X", hr);
                goto End;
            }

            delete pComponent;
            pComponent = nullptr;

            hr = ::CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_IBigDriveRegistration, (void**)&pBigDriveRegistration);
            switch (hr)
            {
            case S_OK:
                if (pBigDriveRegistration == nullptr)
                {
                    hr = E_POINTER;
                    s_eventLogger.WriteErrorFormmated(L"CoCreateInstance() failed. HRESULT: 0x%08X", hr);
                    goto End;
                }

                // Call Register() on the IBigDriveRegistration interface
                hr = pBigDriveRegistration->Register();
                if (FAILED(hr))
                {
                    s_eventLogger.WriteErrorFormmated(
                        L"IBigDriveRegistrationRegister() failed. Application: %s, Component: %s HRESULT: 0x%08X",
                        bstrApplicationName, 
                        bstrComponentName, hr);
                }
                break;
            case REGDB_E_CLASSNOTREG:
                // This COM Component Didn't Register Itself Correctly.
                hr = S_OK;
                break;

            case E_NOINTERFACE:
                // Not Every Component Implements IBigDriveRegistration.
                hr = S_OK;
                break;

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

           if (pBigDriveRegistration)
           {
               pBigDriveRegistration->Release();
               pBigDriveRegistration = nullptr;
           }
        }

        if (pComponentCollection != nullptr)
        {
            delete pComponentCollection;
            pComponentCollection = nullptr;
        }

        if (bstrApplicationName)
        {
            ::SysFreeString(bstrApplicationName);
            bstrApplicationName = nullptr;
        }

        if (pApplication != nullptr)
        {
            delete pApplication;
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

