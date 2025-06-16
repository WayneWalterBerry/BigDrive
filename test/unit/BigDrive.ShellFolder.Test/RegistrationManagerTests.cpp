// <copyright file="RegistrationManagerTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <windows.h>

#include "CppUnitTest.h"

#include "..\..\..\src\BigDrive.ShellFolder\Exports\RegistrationManagerImports.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(RegistrationManagerTests)
    {
        /// <summary>
        /// Test UnregisterShellFolders to ensure it removes all shell folders associated with "BigDrive".
        /// </summary>
        TEST_METHOD(CleanUpShellFolders)
        {
            HRESULT hr = S_OK;

            // Arrange
            GUID bigDriveGuid1 = { 0x12345678, 0x1234, 0x5678, { 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78 } };
            GUID bigDriveGuid2 = { 0x87654321, 0x4321, 0x8765, { 0x98, 0xBA, 0xDC, 0xFE, 0x21, 0x43, 0x65, 0x87 } };

            // Allocate BSTRs properly
            BSTR name1 = ::SysAllocString(L"BigDriveFolder1");
            BSTR name2 = ::SysAllocString(L"BigDriveFolder2");

            // Simulate registry entries for BigDrive shell folders

            hr = ::RegisterShellFolder(bigDriveGuid1, name1);
            Assert::AreEqual(S_OK, hr, L"Failed to register BigDriveFolder1.");

            hr = ::RegisterShellFolder(bigDriveGuid2, name2);
            Assert::AreEqual(S_OK, hr, L"Failed to register BigDriveFolder2.");

            // Act
            hr = ::CleanUpShellFolders();

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

            ::SysFreeString(name1);
            ::SysFreeString(name2);
        }

        /// <summary>
        /// Test RegisterShellFolder to ensure it creates all required registry entries.
        /// </summary>
        TEST_METHOD(RegisterShellFolder)
        {
            HRESULT hr = S_OK;

            // Arrange
            GUID testGuid = { 0xAABBCCDD, 0x1122, 0x3344, { 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC } };
            BSTR testName = ::SysAllocString(L"TestBigDriveShellFolder");

            // Act
            hr = ::RegisterShellFolder(testGuid, testName);

            // Assert
            Assert::AreEqual(S_OK, hr, L"RegisterShellFolder failed.");

            WCHAR guidString[39];
            StringFromGUID2(testGuid, guidString, ARRAYSIZE(guidString));

            // 1. CLSID\{guid} exists and has correct default value
            std::wstring clsidPath = L"CLSID\\" + std::wstring(guidString);
            HKEY hKey = nullptr;
            LONG result = RegOpenKeyEx(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"CLSID key was not created.");

            WCHAR value[256] = {};
            DWORD valueSize = sizeof(value);
            result = RegQueryValueEx(hKey, nullptr, nullptr, nullptr, (LPBYTE)value, &valueSize);
            Assert::AreEqual(ERROR_SUCCESS, result, L"CLSID default value not set.");
            Assert::AreEqual(std::wstring(L"TestBigDriveShellFolder"), std::wstring(value), L"CLSID default value incorrect.");
            RegCloseKey(hKey);

            // 2. CLSID\{guid}\InprocServer32 exists and has correct values
            std::wstring inprocPath = clsidPath + L"\\InprocServer32";
            result = RegOpenKeyEx(HKEY_CLASSES_ROOT, inprocPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"InprocServer32 key was not created.");

            valueSize = sizeof(value);
            result = RegQueryValueEx(hKey, nullptr, nullptr, nullptr, (LPBYTE)value, &valueSize);
            Assert::AreEqual(ERROR_SUCCESS, result, L"InprocServer32 default value not set.");
            // Not asserting the exact DLL path, just that it's non-empty
            Assert::IsTrue(wcslen(value) > 0, L"InprocServer32 default value is empty.");

            valueSize = sizeof(value);
            result = RegQueryValueEx(hKey, L"ThreadingModel", nullptr, nullptr, (LPBYTE)value, &valueSize);
            Assert::AreEqual(ERROR_SUCCESS, result, L"ThreadingModel not set.");
            Assert::AreEqual(std::wstring(L"Apartment"), std::wstring(value), L"ThreadingModel value incorrect.");
            RegCloseKey(hKey);

            // 3. CLSID\{guid}\Implemented Categories\{00021490-0000-0000-C000-000000000046} exists
            std::wstring catPath = clsidPath + L"\\Implemented Categories\\{00021490-0000-0000-C000-000000000046}";
            result = RegOpenKeyEx(HKEY_CLASSES_ROOT, catPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"Implemented Categories key was not created.");
            RegCloseKey(hKey);

            // 4. CLSID\{guid}\ShellFolder exists and has correct values
            std::wstring shellFolderPath = clsidPath + L"\\ShellFolder";
            result = RegOpenKeyEx(HKEY_CLASSES_ROOT, shellFolderPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"ShellFolder key was not created.");

            DWORD attributes = 0;
            DWORD attrSize = sizeof(attributes);
            result = RegQueryValueEx(hKey, L"Attributes", nullptr, nullptr, (LPBYTE)&attributes, &attrSize);
            Assert::AreEqual(ERROR_SUCCESS, result, L"ShellFolder Attributes not set.");
            Assert::IsTrue((attributes & 0xf0800008) == 0xf0800008, L"ShellFolder Attributes value unexpected.");
            valueSize = sizeof(value);
            result = RegQueryValueEx(hKey, L"FolderType", nullptr, nullptr, (LPBYTE)value, &valueSize);
            Assert::AreEqual(ERROR_SUCCESS, result, L"ShellFolder FolderType not set.");
            Assert::AreEqual(std::wstring(L"Storage"), std::wstring(value), L"ShellFolder FolderType value incorrect.");
            RegCloseKey(hKey);

            // 5. DefaultIcon key exists and has correct value
            std::wstring iconPath = clsidPath + L"\\DefaultIcon";
            result = RegOpenKeyEx(HKEY_CLASSES_ROOT, iconPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"DefaultIcon key was not created.");
            valueSize = sizeof(value);
            result = RegQueryValueEx(hKey, nullptr, nullptr, nullptr, (LPBYTE)value, &valueSize);
            Assert::AreEqual(ERROR_SUCCESS, result, L"DefaultIcon value not set.");
            Assert::AreEqual(std::wstring(L"%SystemRoot%\\System32\\imageres.dll,-30"), std::wstring(value), L"DefaultIcon value incorrect.");
            RegCloseKey(hKey);

            // 6. Namespace key under HKCU exists
            std::wstring nsPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\" + std::wstring(guidString);
            result = RegOpenKeyEx(HKEY_CURRENT_USER, nsPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"Namespace key was not created.");
            RegCloseKey(hKey);

            // 7. Component Category key exists
            std::wstring compCatPath = L"Component Categories\\{00021493-0000-0000-C000-000000000046}\\Implementations\\" + std::wstring(guidString);
            result = RegOpenKeyEx(HKEY_CLASSES_ROOT, compCatPath.c_str(), 0, KEY_READ, &hKey);
            Assert::AreEqual(ERROR_SUCCESS, result, L"Component Category key was not created.");
            RegCloseKey(hKey);

            // Cleanup
            ::CleanUpShellFolders();
            ::SysFreeString(testName);
        }
    };


}