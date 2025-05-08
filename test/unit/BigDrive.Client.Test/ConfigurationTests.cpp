// <copyright file="ConfigurationTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"

#include <wtypes.h>

#include "BigDriveClientConfigurationProvider.h"
#include "BigDriveConfigurationClient.h"
#include "IBigDriveConfiguration.h"
#include "GuidUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(ConfigurationTests)
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

        TEST_METHOD(WriteDriveGuidTest)
        {
            HRESULT hr = S_OK;

            // Arrange
            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            hr = WriteDriveGuid(testGuid, testName);
            Assert::AreEqual(S_OK, hr);

            hr = ReadDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);

            // Clean up
            ::SysFreeString(testName);

            hr = DeleteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);
        }

        TEST_METHOD(GetDriveGuidsTest)
        {
            HRESULT hr = S_OK;

            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            hr = WriteDriveGuid(testGuid, testName);
            Assert::AreEqual(S_OK, hr);

            GUID* pGuids = nullptr;

            BigDriveClientConfigurationProvider::GetDriveGuids(&pGuids);

            // Verify that testGuid is in the array pGuids
            bool guidFound = false;
            for (size_t i = 0; pGuids[i] != GUID_NULL; ++i)
            {
                if (IsEqualGUID(pGuids[i], testGuid))
                {
                    guidFound = true;
                    break;
                }
            }

            Assert::IsTrue(guidFound, L"testGuid was not found in the array pGuids.");

            // Clean up
            ::SysFreeString(testName);

            hr = DeleteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);
        }

        TEST_METHOD(GetConfigurationTest)
        {
            // Arrange
            HRESULT hr = S_OK;

            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF2 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            LPWSTR pszConfiguration = nullptr;

            hr = WriteDriveGuid(testGuid, testName);
            Assert::AreEqual(S_OK, hr);

            hr = BigDriveConfigurationClient::GetDriveConfiguration(testGuid, &pszConfiguration);
            Assert::AreEqual(S_OK, hr);

            // Verify the configuration string
            Assert::IsNotNull(pszConfiguration);
            Assert::AreEqual(L"{\"id\":\"12345678-1234-1234-1234-56789abcdef2\",\"name\":\"TestDrive\"}", pszConfiguration);

            // Clean up
            ::SysFreeString(testName);

            ::SysFreeString(pszConfiguration);
            hr = DeleteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);
        }

        TEST_METHOD(GetDriveConfigurationTest)
        {
            // Arrange
            HRESULT hr = S_OK;
            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF2 } };
            BSTR testName = ::SysAllocString(L"TestDrive");

            DriveConfiguration driveConfiguration;

            hr = WriteDriveGuid(testGuid, testName);
            Assert::AreEqual(S_OK, hr);

            hr = BigDriveConfigurationClient::GetDriveConfiguration(testGuid, driveConfiguration);
            Assert::AreEqual(S_OK, hr);

            // Verify the configuration string
            Assert::IsTrue(IsEqualGUID(driveConfiguration.id, testGuid));
            Assert::IsTrue(::wcscmp(testName, driveConfiguration.name) == 0);

            // Clean up
            ::SysFreeString(testName);

            hr = DeleteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);
        }

    private:

        /// <summary>
        /// Helper function to write a drive GUID to the registry.
        /// </summary>
        HRESULT WriteDriveGuid(const GUID& guid, BSTR szName)
        {
            HRESULT hrReturn = S_OK;
            HKEY hKey = nullptr;
            HKEY hSubKey = nullptr;

            // Define the registry path
            const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

            // Convert the GUID to a string
            wchar_t guidString[39]; 
            
            // GUID string format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
            hrReturn = StringFromGUID(guid, guidString, ARRAYSIZE(guidString));
            if (FAILED(hrReturn))
            {
                goto End;
            }

            // Open or create the registry key
            LONG result = ::RegCreateKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            // Create a subkey for the GUID
            result = ::RegCreateKeyEx(hKey, guidString, 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            // Write the GUID value to the subkey
            result = ::RegSetValueEx(hSubKey, L"Id", 0, REG_SZ, reinterpret_cast<const BYTE*>(guidString), (DWORD)((wcslen(guidString) + 1) * sizeof(wchar_t)));
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            /// Write the name value to the subkey
            result = ::RegSetValueEx(hSubKey, L"Name", 0, REG_SZ, reinterpret_cast<const BYTE*>(szName), (DWORD)((wcslen(szName) + 1) * sizeof(wchar_t)));
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

        End:

            // Close the keys
            if (hSubKey != nullptr)
            {
                ::RegCloseKey(hSubKey);
            }

            if (hKey != nullptr)
            {
                ::RegCloseKey(hKey);
            }

            return S_OK;
        }

        /// <summary>
        /// Helper function to read drive GUID from the registry.
        /// </summary>
        HRESULT ReadDriveGuid(GUID& guid)
        {
            HRESULT hrReturn = S_OK;
            HKEY hKey = nullptr;
            DWORD index = 0;
            WCHAR subKeyName[256];
            DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);

            // Define the registry path
            const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

            // Open the registry key
            LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_READ, &hKey);
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            // Enumerate the first subkey (assuming only one GUID is stored)
            result = RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr);
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            // Convert the subkey name (GUID string) back to a GUID
            hrReturn = GUIDFromString(subKeyName, &guid);
            if (FAILED(hrReturn))
            {
                hrReturn = E_INVALIDARG; // Failed to parse GUID
                goto End;
            }

        End:

            // Close the registry key
            if (hKey != nullptr)
            {
                ::RegCloseKey(hKey);
            }

            return hrReturn;
        }

        HRESULT DeleteDriveGuid(const GUID& guid)
        {
            HRESULT hrReturn = S_OK;
            HKEY hKey = nullptr;

            // Define the registry path
            const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

            // Convert the GUID to a string
            wchar_t guidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
            hrReturn = StringFromGUID(guid, guidString, ARRAYSIZE(guidString));
            {
                // Failed to convert GUID to string
                goto End;
            }

            // Open the registry key
            LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, KEY_WRITE, &hKey);
            if (result != ERROR_SUCCESS)
            {
                // If the key doesn't exist, return success
                if (result == ERROR_FILE_NOT_FOUND)
                {
                    hrReturn = S_OK;
                    goto End;
                }

                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            // Delete the subkey for the GUID
            result = RegDeleteKey(hKey, guidString);
            if (result != ERROR_SUCCESS)
            {
                // If the subkey doesn't exist, return success
                if (result == ERROR_FILE_NOT_FOUND)
                {
                    hrReturn = S_OK;
                    goto End;
                }

                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

        End:

            // Close the registry key
            if (hKey != nullptr)
            {
                ::RegCloseKey(hKey);
            }

            return hrReturn;
        }

        HRESULT VerifyRegistryPathForClsid(const GUID& guid)
        {
            HRESULT hrReturn = S_OK;
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
                hrReturn = S_OK;
            }
            else if (result == ERROR_FILE_NOT_FOUND)
            {
                // The registry key does not exist  
                Assert::IsTrue(false, L"Registry path does not exist.");
                hrReturn = S_FALSE;
            }
            else
            {
                // Some other error occurred  
                hrReturn = HRESULT_FROM_WIN32(result);
                Assert::Fail(L"An unexpected error occurred while verifying the registry path.");
            }

            // Close the registry key if it was opened  
            if (hKey != nullptr)
            {
                RegCloseKey(hKey);
            }

            return hrReturn;
        }
        
    };
}
