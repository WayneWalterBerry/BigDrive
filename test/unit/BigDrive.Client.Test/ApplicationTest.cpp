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
#include "IBigDriveConfiguration.h"
#include "GuidUtil.h"
#include "Dispatch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(ApplicationTests)
    {
        TEST_METHOD(GetNameTest)
        {
            HRESULT hrReturn = S_OK;

            IDispatch* pDispatch = nullptr;

            hrReturn = ApplicationManager::GetApplicationsCollection(&pDispatch);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            ApplicationCollection applicationCollection = ApplicationCollection(pDispatch);

            hrReturn = applicationCollection.Initialize();
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Populate() failed.");

            LONG lCount;
            hrReturn = applicationCollection.GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetCount() failed.");
            Assert::IsTrue(lCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hrReturn = applicationCollection.GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetItem() failed.");

            BSTR bstrName;
            hrReturn = pApplication->GetName(bstrName);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetName() failed.");

            ::SysFreeString(bstrName);
        }

        TEST_METHOD(GetComponentsCLSIDsTest)
        {
            HRESULT hrReturn = S_OK;

            IDispatch* pDispatch = nullptr;

            hrReturn = ApplicationManager::GetApplicationsCollection(&pDispatch);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetApplicationsCollection() failed.");

            ApplicationCollection applicationCollection = ApplicationCollection(pDispatch);

            hrReturn = applicationCollection.Initialize();
            Assert::IsTrue(SUCCEEDED(hrReturn), L"Populate() failed.");

            LONG lCount;
            hrReturn = applicationCollection.GetCount(lCount);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetCount() failed.");
            Assert::IsTrue(lCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hrReturn = applicationCollection.GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetItem() failed.");

            CLSID* pclsid = nullptr;
            LONG lSize;
            hrReturn = pApplication->GetComponentsCLSIDs(&pclsid, lSize);
            Assert::IsTrue(SUCCEEDED(hrReturn), L"GetComponentsCLSIDs() failed.");

            if (pclsid != nullptr)
            {
                ::CoTaskMemFree(pclsid);
                pclsid = nullptr;
            }   

            if (pDispatch != nullptr)
            {
                pDispatch->Release();
                pDispatch = nullptr;
            }
        }
    };
}