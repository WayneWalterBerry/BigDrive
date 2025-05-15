// <copyright file="BigDriveClientConfigurationManagerTests.cs" company="Wayne Walter Berry">
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
        /// Make sure that the BigDriveConfiguration COM object is registered correctly.
        /// </summary>
        TEST_METHOD(VerifyClsids)
        {
            HRESULT hr = S_OK;

            // reg query "HKCR\CLSID\{E6F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E7F}"
            hr = VerifyRegistryPathForClsid(CLSID_BigDriveConfiguration);

            Assert::AreEqual(S_OK, hr);
        }

        /// <summary>
        /// Tests the WriteDriveGuid method with valid inputs and verifies the written GUID.
        /// </summary>
        TEST_METHOD(WriteDriveGuidTest)
        {
            HRESULT hr = S_OK;

            // Arrange
            GUID driveGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            GUID clsid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x01, 0xF2 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            hr = BigDriveClientConfigurationManager::WriteDriveGuid(driveGuid, testName, clsid);
            Assert::AreEqual(S_OK, hr);

            hr = BigDriveClientConfigurationManager::ReadDriveGuid(driveGuid);
            Assert::AreEqual(S_OK, hr);

            // Clean up
            ::SysFreeString(testName);

            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(driveGuid);
            Assert::AreEqual(S_OK, hr);
        }

        /// <summary>
        /// Test WriteDriveGuid with an invalid GUID.
        /// </summary>
        TEST_METHOD(TestInvalidDriveGuid)
        {
            // Arrange
            GUID invalidGuid = GUID_NULL; // Invalid GUID
            CLSID validClsid = { 0x87654321, 0x4321, 0x8765, { 0x98, 0xBA, 0xDC, 0xFE, 0x21, 0x43, 0x65, 0x87 } };
            BSTR validName = SysAllocString(L"TestDrive");

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteDriveGuid(invalidGuid, validName, validClsid);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"WriteDriveGuid should fail with an invalid GUID.");

            // Cleanup
            SysFreeString(validName);

        }

        /// <summary>
        /// Test WriteDriveGuid with a null name.
        /// </summary>
        TEST_METHOD(TestNullDriveName)
        {
            BigDriveClientConfigurationManager::DeleteAllDriveGuids();

            // Arrange
            GUID validGuid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            CLSID validClsid = { 0x87654321, 0x4321, 0x8765, { 0x98, 0xBA, 0xDC, 0xFE, 0x21, 0x43, 0x65, 0x87 } };

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteDriveGuid(validGuid, nullptr, validClsid);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"WriteDriveGuid should fail with a null name.");

            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(validGuid);
            Assert::AreEqual(S_OK, hr);
        }

        /// <summary>
        /// Test WriteDriveGuid with an invalid CLSID.
        /// </summary>
        TEST_METHOD(TestInvalidClsid)
        {
            BigDriveClientConfigurationManager::DeleteAllDriveGuids();

            // Arrange
            GUID validGuid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            CLSID invalidClsid = GUID_NULL; // Invalid CLSID
            BSTR validName = SysAllocString(L"TestDrive");

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteDriveGuid(validGuid, validName, invalidClsid);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"WriteDriveGuid should fail with an invalid CLSID.");

            // Cleanup
            SysFreeString(validName);

            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(validGuid);
            Assert::AreEqual(S_OK, hr);
        }

        /// <summary>
        /// Test WriteDriveGuid with a very long name.
        /// </summary>
        TEST_METHOD(TestLongDriveName)
        {
            BigDriveClientConfigurationManager::DeleteAllDriveGuids();

            // Arrange
            GUID validGuid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            CLSID validClsid = { 0x87654321, 0x4321, 0x8765, { 0x98, 0xBA, 0xDC, 0xFE, 0x21, 0x43, 0x65, 0x87 } };
            std::wstring longName(1024, L'A'); // Create a long name with 1024 'A' characters
            BSTR longNameBstr = SysAllocString(longName.c_str());

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteDriveGuid(validGuid, longNameBstr, validClsid);

            // Assert
            Assert::AreEqual(S_OK, hr, L"WriteDriveGuid should succeed with a very long name.");

            // Cleanup
            ::SysFreeString(longNameBstr);

            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(validGuid);
            Assert::AreEqual(S_OK, hr);
        }

        /// <summary>
        /// Tests the GetDriveGuids method to ensure it retrieves all stored drive GUIDs.
        /// </summary>
        TEST_METHOD(GetDriveGuidsTest)
        {
            HRESULT hr = S_OK;

            BigDriveClientConfigurationManager::DeleteAllDriveGuids();

            GUID driveGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1 } };
            GUID clsid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x01, 0xF2 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            hr = BigDriveClientConfigurationManager::WriteDriveGuid(driveGuid, testName, clsid);
            Assert::AreEqual(S_OK, hr);

            GUID* pGuids = nullptr;
            DWORD dwSize = 0;

            hr = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(1 == dwSize, L"Expected one GUID in the array.");

            // Verify that testGuid is in the array pGuids
            bool guidFound = false;
            for (size_t i = 0; i < dwSize; ++i)
            {
                if (IsEqualGUID(pGuids[i], driveGuid))
                {
                    guidFound = true;
                    break;
                }
            }

            Assert::IsTrue(guidFound, L"testGuid was not found in the array pGuids.");

            // Clean up
            ::SysFreeString(testName);

            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(driveGuid);
            Assert::AreEqual(S_OK, hr);
        }

        TEST_METHOD(TestGetDriveGuidsEmpty)
        {
            HRESULT hr = S_OK;

            // Arrange
            BigDriveClientConfigurationManager::DeleteAllDriveGuids();

            // Act
            GUID* pGuids = nullptr;
            DWORD dwSize;

            hr = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(0 == dwSize);
        }

        /// <summary>
        /// Tests the GetDriveConfiguration method to ensure it retrieves the correct configuration for a drive.
        /// </summary>
        TEST_METHOD(GetConfigurationTest)
        {
            // Arrange
            HRESULT hr = S_OK;

            BigDriveClientConfigurationManager::DeleteAllDriveGuids();

            GUID driveGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x00, 0xF2 } };
            GUID clsid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x01, 0xF2 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            LPWSTR pszConfiguration = nullptr;

            hr = BigDriveClientConfigurationManager::WriteDriveGuid(driveGuid, testName, clsid);
            Assert::AreEqual(S_OK, hr);

            hr = BigDriveConfigurationClient::GetDriveConfiguration(driveGuid, &pszConfiguration);
            Assert::AreEqual(S_OK, hr);

            // Verify the configuration string
            Assert::IsNotNull(pszConfiguration);

            // Valid Json Doesn't Have Brackets Around the GUID
            Assert::AreEqual(
                L"{\"id\":\"12345678-1234-1234-1234-56789abc00f2\",\"name\":\"TestDrive\",\"clsid\":\"12345678-1234-1234-1234-56789abc01f2\"}",
                pszConfiguration);

            // Clean up
            ::SysFreeString(testName);

            ::SysFreeString(pszConfiguration);
            hr = BigDriveClientConfigurationManager::DeleteDriveGuid(driveGuid);
            Assert::AreEqual(S_OK, hr);
        }

        /// <summary>
        /// Test WriteProviderClsId with valid CLSID and name.
        /// </summary>
        TEST_METHOD(TestValidProviderClsId)
        {
            // Arrange
            CLSID validClsid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            BSTR validName = SysAllocString(L"TestProvider");

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteProviderClsId(validClsid, validName);

            // Assert
            Assert::AreEqual(S_OK, hr, L"WriteProviderClsId should succeed with valid inputs.");

            // Cleanup
            SysFreeString(validName);
        }

        /// <summary>
        /// Test WriteProviderClsId with an invalid CLSID.
        /// </summary>
        TEST_METHOD(TestInvalidProviderClsId)
        {
            // Arrange
            CLSID invalidClsid = GUID_NULL; // Invalid CLSID
            BSTR validName = SysAllocString(L"TestProvider");

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteProviderClsId(invalidClsid, validName);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"WriteProviderClsId should fail with an invalid CLSID.");

            // Cleanup
            SysFreeString(validName);
        }

        /// <summary>
        /// Test WriteProviderClsId with a null name.
        /// </summary>
        TEST_METHOD(TestNullProviderName)
        {
            // Arrange
            CLSID validClsid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteProviderClsId(validClsid, nullptr);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"WriteProviderClsId should fail with a null name.");
        }

        /// <summary>
        /// Test WriteProviderClsId with a very long name.
        /// </summary>
        TEST_METHOD(TestLongProviderName)
        {
            // Arrange
            CLSID validClsid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            std::wstring longName(1024, L'A'); // Create a long name with 1024 'A' characters
            BSTR longNameBstr = SysAllocString(longName.c_str());

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::WriteProviderClsId(validClsid, longNameBstr);

            // Assert
            Assert::AreEqual(S_OK, hr, L"WriteProviderClsId should succeed with a very long name.");

            // Cleanup
            SysFreeString(longNameBstr);
        }

        /// <summary>
        /// Tests the CleanDrives method to ensure it removes drives associated with unregistered providers
        /// while retaining drives linked to registered providers. Verifies the integrity of the remaining
        /// drive GUIDs after cleanup.
        /// </summary>
        TEST_METHOD(TestCleanDrives)
        {
            HRESULT hr = S_OK;

            // Arrange
            GUID guidDrive1 = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x00, 0x00 } };
            GUID guidDrive2 = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x00, 0x01 } };
            CLSID clsidRegisteredProvider = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            CLSID clsidUnRegisteredProvider = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x79 } };

            hr = BigDriveClientConfigurationManager::WriteProviderClsId(clsidRegisteredProvider, L"ValidProvider");
            Assert::AreEqual(S_OK, hr, L"WriteProviderClsId should Not Fail");

            hr = BigDriveClientConfigurationManager::WriteDriveGuid(guidDrive1, L"Drive 1", clsidRegisteredProvider);
            Assert::AreEqual(S_OK, hr, L"WriteDriveGuid should Not Fail");
            hr = BigDriveClientConfigurationManager::WriteDriveGuid(guidDrive2, L"Drive 2", clsidUnRegisteredProvider);
            Assert::AreEqual(S_OK, hr, L"WriteDriveGuid should Not Fail");

            // Act
            hr = BigDriveClientConfigurationManager::CleanDrives();
            Assert::AreEqual(S_OK, hr, L"CleanDrives should Not Fail");

            // Assert
            GUID* pGuids = nullptr;
            DWORD dwSize = 0;

            hr = BigDriveClientConfigurationManager::GetDriveGuids(&pGuids, dwSize);
            Assert::AreEqual(S_OK, hr);

            // Verify that guidDrive1 is in the array pGuids
            bool guidFound = false;
            for (size_t i = 0; i < dwSize; ++i)
            {
                if (IsEqualGUID(pGuids[i], guidDrive1))
                {
                    guidFound = true;
                    break;
                }

                Assert::IsFalse(IsEqualGUID(pGuids[i], guidDrive2) == 0, L"guidDrive2 was supposed to be deleted");
            }

            Assert::IsTrue(guidFound, L"guidDrive1 was not found in the array pGuids.");

            // Cleanup
            if (pGuids == nullptr)
            {
                CoTaskMemFree(pGuids);
                pGuids = nullptr;
            }
        }

        /// <summary>
        /// Test DoesProviderSubkeyExist when the provider subkey exists.
        /// </summary>
        TEST_METHOD(TestProviderSubkeyExists)
        {
            // Arrange
            CLSID validClsid = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            BSTR validName = SysAllocString(L"TestProvider");

            HRESULT hr = BigDriveClientConfigurationManager::WriteProviderClsId(validClsid, validName);
            Assert::AreEqual(S_OK, hr, L"WriteProviderClsId should succeed.");

            // Act
            hr = BigDriveClientConfigurationManager::DoesProviderSubkeyExist(validClsid);

            // Assert
            Assert::AreEqual(S_OK, hr, L"DoesProviderSubkeyExist should return S_OK for an existing subkey.");

            // Cleanup
            SysFreeString(validName);
        }

        /// <summary>
        /// Test DoesProviderSubkeyExist when the provider subkey does not exist.
        /// </summary>
        TEST_METHOD(TestProviderSubkeyDoesNotExist)
        {
            // Arrange
            CLSID nonExistentClsid = { 0x87654321, 0x4321, 0x8765, { 0x98, 0xBA, 0xDC, 0xFE, 0x21, 0x43, 0x65, 0x87 } };

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::DoesProviderSubkeyExist(nonExistentClsid);

            // Assert
            Assert::AreEqual(S_FALSE, hr, L"DoesProviderSubkeyExist should return S_FALSE for a non-existent subkey.");
        }

        /// <summary>
        /// Test DoesProviderSubkeyExist with an invalid CLSID.
        /// </summary>
        TEST_METHOD(TestInvalidProviderClsid)
        {
            // Arrange
            CLSID invalidClsid = GUID_NULL; // Invalid CLSID

            // Act
            HRESULT hr = BigDriveClientConfigurationManager::DoesProviderSubkeyExist(invalidClsid);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"DoesProviderSubkeyExist should fail with an invalid CLSID.");
        }

    private:

        HRESULT VerifyRegistryPathForClsid(const GUID& guid)
        {
            HRESULT hr = S_OK;
            HKEY hKey = nullptr;

            // Convert the GUID to a string  
            wchar_t guidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}  
            if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
            {
                return E_FAIL; // Failed to convert GUID to string  
            }

            // Construct the registry path  
            std::wstring registryPath = L"CLSID\\";
            registryPath += guidString;

            // Try to open the registry key  
            LONG result = RegOpenKeyEx(HKEY_CLASSES_ROOT, registryPath.c_str(), 0, KEY_READ, &hKey);
            if (result == ERROR_SUCCESS)
            {
                // The registry key exists  
                Assert::IsTrue(true, L"Registry path exists.");
                hr = S_OK;
            }
            else if (result == ERROR_FILE_NOT_FOUND)
            {
                // The registry key does not exist  
                Assert::IsTrue(false, L"Registry path does not exist.");
                hr = S_FALSE;
            }
            else
            {
                // Some other error occurred  
                hr = HRESULT_FROM_WIN32(result);
                Assert::Fail(L"An unexpected error occurred while verifying the registry path.");
            }

            // Close the registry key if it was opened  
            if (hKey != nullptr)
            {
                RegCloseKey(hKey);
            }

            return hr;
        }

    };
}
