// <copyright file="ApplicationTests.cpp" company="Wayne Walter Berry">
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
            HRESULT hrReturn = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hrReturn = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hrReturn = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            hrReturn = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Populate() failed.");

            LONG lCount;
            hrReturn = pApplicationCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetCount() failed.");
            Assert::IsTrue(lCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hrReturn = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetItem() failed.");

            BSTR bstrName;
            hrReturn = pApplication->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetName() failed.");
            ::SysFreeString(bstrName);

            BSTR bstrDescription;
            hrReturn = pApplication->GetDescription(bstrDescription);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetDescription() failed.");
            ::SysFreeString(bstrDescription);

            BSTR bstrId;
            hrReturn = pApplication->GetId(bstrId);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetId() failed.");
            ::SysFreeString(bstrId);

            delete pApplicationCollection;
        }

        TEST_METHOD(GetComponentCollectionTest)
        {
            HRESULT hrReturn = S_OK;

            COMAdminCatalog* pCOMAdminCatalog;
            hrReturn = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Create() failed.");

            ApplicationCollection* pApplicationCollection;
            hrReturn = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            hrReturn = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Populate() failed.");

            LONG lCount;
            hrReturn = pApplicationCollection->GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetCount() failed.");
            Assert::IsTrue(lCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hrReturn = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetItem() failed.");

            BSTR bstrTypeLibrary;
            hrReturn = pApplication->GetTypeInfo(bstrTypeLibrary);
            Assert::AreEqual(L"COMAdmin", bstrTypeLibrary);
            ::SysFreeString(bstrTypeLibrary);

            BSTR bstrDescriptions = nullptr;
            pCOMAdminCatalog->FunctionDescriptions(bstrDescriptions);

            BSTR bstrApplId;
            hrReturn = pApplication->GetId(bstrApplId);

            LPDISPATCH pIDispatch;

            pCOMAdminCatalog->GetCollectionByQuery(L"Components", bstrApplId, &pIDispatch);

            ComponentCollection* pComponentCollection = nullptr;
            hrReturn = pCOMAdminCatalog->GetComponentCollection(pApplication, &pComponentCollection);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetComponents() failed.");


            if (pComponentCollection != nullptr)
            {
                delete pComponentCollection;
            }

            delete pApplicationCollection;
        }
    };
}