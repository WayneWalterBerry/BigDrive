// <copyright file="BigDriveConfigurationClientTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"

#include <wtypes.h>

#include "BigDriveClientConfigurationManager.h"
#include "BigDriveConfigurationClient.h"
#include "IBigDriveConfiguration.h"
#include "GuidUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(BigDriveClientConfigurationManagerTests)
    {
    public:
        /// <summary>
        /// Tests the GetDriveConfiguration method to ensure it retrieves the correct configuration for a drive.
        /// </summary>
        TEST_METHOD(GetDriveConfigurationTest)
        {
            // Arrange
            HRESULT hr = S_OK;
            GUID driveGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF2 } };
            GUID clsid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x01, 0xF2 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            DriveConfiguration driveConfiguration;

            hr = BigDriveClientConfigurationManager::WriteDriveGuid(driveGuid, testName, clsid);
            Assert::AreEqual(S_OK, hr);

            hr = BigDriveConfigurationClient::GetDriveConfiguration(driveGuid, driveConfiguration);
            Assert::AreEqual(S_OK, hr);

            // Verify the configuration string
            Assert::IsTrue(IsEqualGUID(driveConfiguration.id, driveGuid));
            Assert::IsTrue(::wcscmp(testName, driveConfiguration.name) == 0);

            // Clean up
            ::SysFreeString(testName);

            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(driveGuid);
            Assert::AreEqual(S_OK, hr);
        }
    };
}