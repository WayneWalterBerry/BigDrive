// <copyright file="ApplicationTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"

// System
#include <windows.h>
#include <wtypes.h>
#include <string.h>

#include "Application.h"
#include "ApplicationCollection.h"
#include "ApplicationManager.h"
#include "BigDriveClientConfigurationManager.h"
#include "BigDriveConfigurationClient.h"
#include "ComponentCollection.h"
#include "COMAdminCatalog.h"
#include "IBigDriveConfiguration.h"
#include "GuidUtil.h"
#include "Dispatch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(ApplicationTests)
    {
        TEST_METHOD(GetValuesTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Populate() failed.");

            LONG lCount;
            hr = pApplicationCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hr = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");

            BSTR bstrName;
            hr = pApplication->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hr), L"GetName() failed.");
            ::SysFreeString(bstrName);

            BSTR bstrDescription;
            hr = pApplication->GetDescription(bstrDescription);
            Assert::IsTrue(SUCCEEDED(hr), L"GetDescription() failed.");
            ::SysFreeString(bstrDescription);

            BSTR bstrId;
            hr = pApplication->GetId(bstrId);
            Assert::IsTrue(SUCCEEDED(hr), L"GetId() failed.");
            ::SysFreeString(bstrId);

            if (pCOMAdminCatalog != nullptr)
            {
                delete pCOMAdminCatalog;
                pCOMAdminCatalog = nullptr;
            }

            if (pApplicationCollection != nullptr)
            {
                delete pApplicationCollection;
                pApplicationCollection = nullptr;
            }

            if (pApplication != nullptr)
            {
                delete pApplication;
                pApplication = nullptr;
            }
        }

        TEST_METHOD(GetComponentCollectionTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Populate() failed.");

            LONG lApplicationCount;
            hr = pApplicationCollection->GetCount(lApplicationCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lApplicationCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hr = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");

            ComponentCollection* pComponentCollection = nullptr;
            hr = pCOMAdminCatalog->GetComponentCollection(pApplication, &pComponentCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetComponents() failed.");

            LONG lComponentCount;
            hr = pComponentCollection->GetCount(lComponentCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lComponentCount > 0, L"Expected at least one component.");

            if (pCOMAdminCatalog != nullptr)
            {
                delete pCOMAdminCatalog;
                pCOMAdminCatalog = nullptr;
            }

            if (pComponentCollection != nullptr)
            {
                delete pComponentCollection;
                pComponentCollection = nullptr;
            }

            if (pApplicationCollection != nullptr)
            {
                delete pApplicationCollection;
                pApplicationCollection = nullptr;
            }

            if (pComponentCollection != nullptr)
            {
                delete pComponentCollection;
                pComponentCollection = nullptr;
            }
        }

        TEST_METHOD(GetICatalogObjectTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");
            Assert::IsNotNull(pApplicationCollection, L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize() failed.");

            LONG lCount;
            hr = pApplicationCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lCount > 0, L"Expected at least one application.");

            Application *pApplication;
            hr = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");
            Assert::IsNotNull(pApplication, L"GetItem() failed.");

            ICatalogObject* pCatalogColection;
            hr = pApplication->GetICatalogObject(&pCatalogColection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetICatalogObject() failed.");
            Assert::IsNotNull(pCatalogColection, L"GetICatalogObject() failed.");

            if (pCOMAdminCatalog != nullptr)
            {
                delete pCOMAdminCatalog;
                pCOMAdminCatalog = nullptr;
            }

            if (pApplicationCollection != nullptr)
            {
                delete pApplicationCollection;
                pApplicationCollection = nullptr;
            }

            if (pApplication != nullptr)
            {
                delete pApplication;
                pApplication = nullptr;
            }

            if (pCatalogColection != nullptr)
            {
                pCatalogColection->Release();
                pCatalogColection = nullptr;
            }
        }
    };
}
