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
#include "Dispatch.h"

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

        /// <summary>
        /// Test case for RegisterApplications
        /// </summary>
        TEST_METHOD(RegisterApplications_Basic)
        {
            // Act
            HRESULT hr = ApplicationManager::RegisterApplications();

            // Assert
            Assert::IsTrue(SUCCEEDED(hr), L"RegisterApplications() failed.");
        }
    };
}
