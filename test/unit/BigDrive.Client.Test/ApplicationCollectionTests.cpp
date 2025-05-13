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
#include "GuidUtil.h"
#include "Dispatch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(ApplicationCollectionTests)
    {

    private:

        TEST_METHOD(GetApplicationsCollection_ValidInput)
        {
            IDispatch* pDispatch = nullptr;

            HRESULT hrReturn = ApplicationManager::GetApplicationsCollection(&pDispatch);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            if (pDispatch != nullptr)
            {
                pDispatch->Release();
                pDispatch = nullptr;
            }
        }

        TEST_METHOD(GetApplicationsCollection_Populate)
        {
            HRESULT hrReturn = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hrReturn = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Create() failed.");

            ApplicationCollection *pApplicationCollection;
            hrReturn = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            hrReturn = pApplicationCollection->Populate();
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Populate() failed.");

            LONG lCount;
            hrReturn = pApplicationCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetCount() failed.");

            BSTR bstrName;
            hrReturn = pApplicationCollection->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetName() failed.");
            Assert::AreEqual(bstrName, L"Applications");

            if (bstrName != nullptr)
            {
                ::SysFreeString(bstrName);
                bstrName = nullptr;
            }

            delete pApplicationCollection;
        }

        TEST_METHOD(GetApplications)
        {
            HRESULT hrReturn = S_OK;

            // Arrange
            COMAdminCatalog* pCOMAdminCatalog;
            hrReturn = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hrReturn = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            // Act
            Application** ppApplications = nullptr;
            LONG lSize = 0;
            hrReturn = pApplicationCollection->GetApplications(&ppApplications, lSize);

            // Assert
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplications() failed.");

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
        }

    };
}