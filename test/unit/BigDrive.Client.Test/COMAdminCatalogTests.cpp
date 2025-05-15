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
        TEST_METHOD(Start_ValidApplication_Succeeds)
        {
            HRESULT hr = S_OK;

            // Arrange: Create COMAdminCatalog and get an Application
            COMAdminCatalog* pCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"COMAdminCatalog::Create failed.");
            Assert::IsNotNull(pCatalog, L"pCatalog is null.");

            ApplicationCollection* pAppCollection = nullptr;
            hr = pCatalog->GetApplicationsCollection(&pAppCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection failed.");
            Assert::IsNotNull(pAppCollection, L"pAppCollection is null.");

            LONG lCount = 0;
            hr = pAppCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount failed.");
            Assert::IsTrue(lCount > 0, L"No applications found.");

            Application* pApp = nullptr;
            hr = pAppCollection->GetItem(0, &pApp);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem failed.");
            Assert::IsNotNull(pApp, L"pApp is null.");

            // Act: Start the application
            hr = pCatalog->Start(pApp);

            // Assert: Should succeed or return a documented COM+ error
            Assert::IsTrue(SUCCEEDED(hr), L"Start failed.");

            // Cleanup
            delete pApp;
            delete pAppCollection;
            delete pCatalog;
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