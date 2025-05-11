// <copyright file="RegistrationManagerTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "CppUnitTest.h"

#include "dllmain.h"

#include "BigDriveShellFolderFactory.h"
#include "RegistrationManager.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    /// <summary>
    /// Unit tests for the DllGetClassObject function in dllmain.cpp.
    /// </summary>
    TEST_CLASS(RegistrationManagerTests)
    {
        /// <summary>
        /// Test UnregisterShellFolders to ensure it removes all shell folders associated with "BigDrive".
        /// </summary>
        TEST_METHOD(TestUnregisterShellFolders)
        {
            HRESULT hr = S_OK;

            // Arrange
            RegistrationManager& manager = RegistrationManager::GetInstance();

            GUID bigDriveGuid1 = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            GUID bigDriveGuid2 = { 0x87654321, 0x4321, 0x8765, { 0x98, 0xBA, 0xDC, 0xFE, 0x21, 0x43, 0x65, 0x87 } };
            WCHAR modulePath[MAX_PATH];

            // Simulate registry entries for BigDrive shell folders
            hr = manager.GetModuleFileNameW(modulePath, MAX_PATH);
            Assert::AreEqual(S_OK, hr, L"Failed to get the Module File Name");

            hr = manager.RegisterShellFolder(bigDriveGuid1, L"BigDriveFolder1");
            Assert::AreEqual(S_OK, hr, L"Failed to register BigDriveFolder1.");

            hr = manager.RegisterShellFolder(bigDriveGuid2, L"BigDriveFolder2");
            Assert::AreEqual(S_OK, hr, L"Failed to register BigDriveFolder2.");

            // Act
            hr = manager.UnregisterShellFolders();

            // Assert
            Assert::AreEqual(S_OK, hr, L"UnregisterShellFolders failed.");

            // Verify that the shell folders were removed
            HKEY hKey = nullptr;
            WCHAR guidString[39];

            if (StringFromGUID2(bigDriveGuid1, guidString, ARRAYSIZE(guidString)) > 0)
            {
                std::wstring clsidPath = L"CLSID\\" + std::wstring(guidString);
                LONG result = RegOpenKeyEx(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, KEY_READ, &hKey);
                Assert::AreEqual(ERROR_FILE_NOT_FOUND, result, L"BigDriveFolder1 was not removed.");
            }

            if (StringFromGUID2(bigDriveGuid2, guidString, ARRAYSIZE(guidString)) > 0)
            {
                std::wstring clsidPath = L"CLSID\\" + std::wstring(guidString);
                LONG result = RegOpenKeyEx(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, KEY_READ, &hKey);
                Assert::AreEqual(ERROR_FILE_NOT_FOUND, result, L"BigDriveFolder2 was not removed.");
            }
        }
    };
}