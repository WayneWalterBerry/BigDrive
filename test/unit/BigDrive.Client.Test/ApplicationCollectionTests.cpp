// <copyright file="ApplicationCollectionTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"

// System
#include <wtypes.h>
#include <string.h>

#include "Application.h"
#include "ApplicationCollection.h"
#include "ApplicationManager.h"
#include "COMAdminCatalog.h"
#include "BigDriveClientConfigurationManager.h"
#include "BigDriveConfigurationClient.h"
#include "IBigDriveConfiguration.h"
#include "Interfaces/ICatalogCollection.h"
#include "GuidUtil.h"
#include "Dispatch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(ApplicationCollectionTests)
    {

    public:

        ApplicationCollectionTests()
        {
            ::EnableMemoryLeakChecks();
        }

    public:

        TEST_METHOD(QueryApplicationByName_FindsApplication)
        {
            HRESULT hr = S_OK;

            // Arrange: Create COMAdminCatalog and get ApplicationCollection
            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"COMAdminCatalog::Create failed.");

            ApplicationCollection* pApplicationCollection = nullptr;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection failed.");
            Assert::IsNotNull(pApplicationCollection, L"pApplicationCollection is null.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize failed.");

            // Act: Query for the application by name
            Application* pFoundApp = nullptr;
            hr = pApplicationCollection->QueryApplicationByName(L"BigDrive.Provider.Sample", &pFoundApp);

            // Assert: Should succeed and return a non-null application pointer
            Assert::IsTrue(SUCCEEDED(hr), L"QueryApplicationByName failed.");
            Assert::IsNotNull(pFoundApp, L"QueryApplicationByName did not return an application.");

            // Optionally, verify the name matches
            BSTR bstrName = nullptr;
            hr = pFoundApp->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hr), L"GetName failed on found application.");
            Assert::IsTrue(bstrName != nullptr, L"GetName returned null.");
            Assert::IsTrue(wcscmp(bstrName, L"BigDrive.Provider.Sample") == 0, L"Returned application name does not match.");

            // Cleanup
            if (bstrName) ::SysFreeString(bstrName);
            delete pFoundApp;
            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(GetApplicationsCollection_Populate)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection *pApplicationCollection;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Populate();
            Assert::IsTrue(SUCCEEDED(hr), L"Populate() failed.");

            LONG lCount;
            hr = pApplicationCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");

            BSTR bstrName;
            hr = pApplicationCollection->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hr), L"GetName() failed.");
            Assert::AreEqual(bstrName, L"Applications");

            if (bstrName != nullptr)
            {
                ::SysFreeString(bstrName);
                bstrName = nullptr;
            }

            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(GetICatalogCollectionTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");
            Assert::IsNotNull(pApplicationCollection, L"GetApplicationsCollection() failed.");

            ICatalogCollection* pICatalogCollection = nullptr;
            hr = pApplicationCollection->GetICatalogCollection(&pICatalogCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");
            Assert::IsNotNull(pICatalogCollection, L"GetApplicationsCollection() failed.");

            delete pApplicationCollection;
            pICatalogCollection->Release();
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(GetApplications)
        {
            HRESULT hr = S_OK;

            // Arrange
            COMAdminCatalog* pCOMAdminCatalog;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            // Act
            Application** ppApplications = nullptr;
            LONG lSize = 0;
            hr = pApplicationCollection->GetApplications(&ppApplications, lSize);

            // Assert
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplications() failed.");

            if (*ppApplications != nullptr)
            {
                for (LONG j = 0; j < lSize; j++)
                {
                    if (ppApplications[j] != nullptr)
                    {
                        delete (ppApplications)[j];
                    }
                }
                ::CoTaskMemFree(ppApplications);
                ppApplications = nullptr;
                lSize = 0;
            }

            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }
    };
}