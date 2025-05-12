// <copyright file="ApplicationManagerTests.cpp" company="Wayne Walter Berry">
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
#include "COM.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(ApplicationManagerTests)
    {

    private:

        // Big Drive Sample Provider CLSID
        const CLSID CLSID_BigDriveSampleProvider = { 0xF8FE2E5A, 0xE8B8, 0x4207, { 0xBC, 0x04, 0xEA, 0x4B, 0xCD, 0x4C, 0x43, 0x61 } };

    public:

        // Test: GetCOMAdminCatalog with valid input
        TEST_METHOD(GetCOMAdminCatalog_ValidInput)
        {
            IDispatch* pDispatch = nullptr;

            HRESULT hr = ApplicationManager::GetCOMAdminCatalog(&pDispatch);

            Assert::IsTrue(SUCCEEDED(hr), L"GetCOMAdminCatalog() failed.");
            Assert::IsTrue(pDispatch != nullptr, L"Expected a valid IDispatch pointer.");
        }



        /// <summary>
        /// Test case for StartApplication with a valid CLSID
        /// </summary>
        TEST_METHOD(StartApplication_BigDriveSampleProvider)
        {
            // Act
            HRESULT hr = ApplicationManager::StartApplication(CLSID_BigDriveSampleProvider);

            // Assert
            Assert::IsTrue(SUCCEEDED(hr), L"StartApplication() failed.");
        }

        /// <summary>
        /// Test case for StartApplication with an invalid CLSID
        /// </summary>
        TEST_METHOD(StartApplication_InvalidCLSID)
        {
            // Arrange
            CLSID invalidClsid = { 0 }; // Invalid CLSID (all zeros)

            // Act
            HRESULT hr = ApplicationManager::StartApplication(invalidClsid);

            // Assert
            Assert::IsTrue(FAILED(hr), L"StartApplication() succeeds.");
        }
    };
}
