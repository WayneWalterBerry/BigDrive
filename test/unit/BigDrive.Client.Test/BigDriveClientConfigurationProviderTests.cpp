#include "pch.h"
#include "CppUnitTest.h"

#include <wtypes.h>

#include "BigDriveClientConfigurationProvider.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveClientTest
{
    TEST_CLASS(BigDriveClientConfigurationProviderTests)
    {
    public:

        TEST_METHOD(WriteDriveGuidTest)
        {
            // Arrange
            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            HRESULT hr = S_OK;

            hr = WriteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);

            hr = ReadDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);

            // Clean up
            hr = DeleteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);
        }

        TEST_METHOD(GetDriveGuidsTest)
        {
            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1 } };
            HRESULT hr = S_OK;

            hr = WriteDriveGuid(testGuid);
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
            hr = DeleteDriveGuid(testGuid);
            Assert::AreEqual(S_OK, hr);
        }

    private:

        /// <summary>
        /// Helper function to write a drive GUID to the registry.
        /// </summary>
        HRESULT WriteDriveGuid(const GUID& guid)
        {
            HRESULT hrReturn = S_OK;
            HKEY hKey = nullptr;
            HKEY hSubKey = nullptr;

            // Define the registry path
            const std::wstring drivesRegistryPath = L"Software\\BigDrive\\Drives";

            // Convert the GUID to a string
            wchar_t guidString[39]; // GUID string format: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
            if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
            {
                hrReturn = E_FAIL; // Failed to convert GUID to string
                goto End;
            }

            // Open or create the registry key
            LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, drivesRegistryPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
            if (result != ERROR_SUCCESS)
            {
                hrReturn = HRESULT_FROM_WIN32(result);
                goto End;
            }

            // Create a subkey for the GUID
            result = RegCreateKeyEx(hKey, guidString, 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);
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
            if (CLSIDFromString(subKeyName, &guid) != S_OK)
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
            if (StringFromGUID2(guid, guidString, ARRAYSIZE(guidString)) == 0)
            {
                // Failed to convert GUID to string
                hrReturn = E_FAIL;
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
    };
}
