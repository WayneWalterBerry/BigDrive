// <copyright file="COMAdminCatalogTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"

// System
#include <wtypes.h>
#include <string.h>

#include "ApplicationCollection.h"
#include "BigDriveClientConfigurationManager.h"
#include "BigDriveConfigurationClient.h"
#include "COMAdminCatalog.h"
#include "ComponentCollection.h"
#include "IBigDriveConfiguration.h"
#include "GuidUtil.h"
#include "Dispatch.h"
#include "Interfaces/ICOMAdminCatalog2.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(COMAdminCatalogTests)
    {
        TEST_METHOD(COMAdminCatalog_QueryApplicationByName_FindsApplication)
        {
            HRESULT hr = S_OK;

            // Arrange: Create COMAdminCatalog
            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"COMAdminCatalog::Create failed.");
            Assert::IsNotNull(pCOMAdminCatalog, L"pCOMAdminCatalog is null.");

            // Act: Query for the application by name
            Application* pBigDriveProviderSampleApplication = nullptr;
            hr = pCOMAdminCatalog->QueryApplicationByName(L"BigDrive.Provider.Sample", &pBigDriveProviderSampleApplication);

            // Assert: Should succeed and return a non-null application pointer
            Assert::IsTrue(SUCCEEDED(hr), L"QueryApplicationByName failed.");
            Assert::IsNotNull(pBigDriveProviderSampleApplication, L"QueryApplicationByName did not return an application.");

            // Optionally, verify the name matches
            BSTR bstrName = nullptr;
            hr = pBigDriveProviderSampleApplication->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hr), L"GetName failed on found application.");
            Assert::IsTrue(bstrName != nullptr, L"GetName returned null.");
            Assert::IsTrue(wcscmp(bstrName, L"BigDrive.Provider.Sample") == 0, L"Returned application name does not match.");

            // Cleanup
            if (bstrName) ::SysFreeString(bstrName);
            delete pBigDriveProviderSampleApplication;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(Start_ValidApplication_Succeeds)
        {
            HRESULT hr = S_OK;

            // Arrange: Create COMAdminCatalog and get an Application
            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"COMAdminCatalog::Create failed.");
            Assert::IsNotNull(pCOMAdminCatalog, L"pCatalog is null.");

            // Act: Query for the application by name
            Application* pBigDriveProviderSampleApplication = nullptr;
            hr = pCOMAdminCatalog->QueryApplicationByName(L"BigDrive.Provider.Sample", &pBigDriveProviderSampleApplication);

            // Assert: Should succeed and return a non-null application pointer
            Assert::IsTrue(SUCCEEDED(hr), L"QueryApplicationByName failed.");
            Assert::IsNotNull(pBigDriveProviderSampleApplication, L"QueryApplicationByName did not return an application.");

            // Act: Start the application
            hr = pCOMAdminCatalog->Start(pBigDriveProviderSampleApplication);

            // Assert: Should succeed or return a documented COM+ error
            Assert::IsTrue(SUCCEEDED(hr), L"Start failed.");

            // Cleanup
            delete pBigDriveProviderSampleApplication;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(TestGetApplicationsCollection)
        {
            // Arrange
            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            HRESULT hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::AreEqual(S_OK, hr, L"Failed to create COMAdminCatalog instance.");
            Assert::IsNotNull(pCOMAdminCatalog, L"COMAdminCatalog instance should not be null.");

            ApplicationCollection* pApplicationCollection = nullptr;

            // Act
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);

            // Assert
            Assert::AreEqual(S_OK, hr, L"GetApplicationsCollection should return S_OK.");
            Assert::IsNotNull(pApplicationCollection, L"ApplicationCollection should not be null.");

            LONG appCount = 0;
            hr = pApplicationCollection->GetCount(appCount);
            Assert::AreEqual(S_OK, hr, L"GetCount should return S_OK.");
            Assert::IsTrue(appCount > 0, L"ApplicationCollection should contain at least one application.");

            // Clean up
            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(TestGetICOMAdminCatalog2)
        {
            // Arrange
            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            HRESULT hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::AreEqual(S_OK, hr, L"Failed to create COMAdminCatalog instance.");
            Assert::IsNotNull(pCOMAdminCatalog, L"COMAdminCatalog instance should not be null.");

            ICOMAdminCatalog2* pICOMAdminCatalog2 = nullptr;
            hr = pCOMAdminCatalog->GetICOMAdminCatalog2(&pICOMAdminCatalog2);
            Assert::AreEqual(S_OK, hr, L"Failed to create ICOMAdminCatalog2 instance.");
            Assert::IsNotNull(pICOMAdminCatalog2, L"ICOMAdminCatalog2 instance should not be null.");

            pCOMAdminCatalog->Release();
            pCOMAdminCatalog = nullptr;
        }
    };
}