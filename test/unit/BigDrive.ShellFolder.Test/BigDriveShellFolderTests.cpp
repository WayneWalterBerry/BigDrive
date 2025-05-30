// <copyright file="BigDriveShellFolderTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <windows.h>
#include <shlobj.h>
#include <comdef.h>

#include "CppUnitTest.h"

#include "..\..\..\src\IShellFolder\Exports\BigDriveShellFolderExports.h"
#include "..\..\..\src\IShellFolder\BigDriveItemType.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    /// <summary>
    /// Unit tests for BigDriveShellFolder PIDL allocation and name extraction.
    /// </summary>
    TEST_CLASS(BigDriveShellFolderTests)
    {
    public:

        /// <summary>
        /// Test AllocateBigDriveItemIdExport with valid file item and verify the name using GetBigDriveItemNameFromPidlExport.
        /// </summary>
        TEST_METHOD(AllocateBigDriveItemIdExport_File)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR name = ::SysAllocString(L"TestFile.txt");
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_File, name, &pidl);

            Assert::AreEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should succeed for file.");
            Assert::IsNotNull(pidl, L"PIDL should not be null for valid file.");

            if (pidl)
            {
                STRRET strret = {};
                hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
                Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for file PIDL.");
                Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
                Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
                Assert::AreEqual(std::wstring(L"TestFile.txt"), std::wstring(strret.pOleStr), L"Extracted file name does not match.");
                ::CoTaskMemFree(strret.pOleStr);
                ::CoTaskMemFree(pidl);
            }
            ::SysFreeString(name);
        }

        /// <summary>
        /// Test AllocateBigDriveItemIdExport with valid folder item and verify the name using GetBigDriveItemNameFromPidlExport.
        /// </summary>
        TEST_METHOD(AllocateBigDriveItemIdExport_Folder)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR name = ::SysAllocString(L"TestFolder");
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_Folder, name, &pidl);

            Assert::AreEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should succeed for folder.");
            Assert::IsNotNull(pidl, L"PIDL should not be null for valid folder.");

            if (pidl)
            {
                STRRET strret = {};
                hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
                Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for folder PIDL.");
                Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
                Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
                Assert::AreEqual(std::wstring(L"TestFolder"), std::wstring(strret.pOleStr), L"Extracted folder name does not match.");
                ::CoTaskMemFree(strret.pOleStr);
                ::CoTaskMemFree(pidl);
            }
            ::SysFreeString(name);
        }

        /// <summary>
        /// Test AllocateBigDriveItemIdExport with null name (should fail).
        /// </summary>
        TEST_METHOD(AllocateBigDriveItemIdExport_NullName)
        {
            LPITEMIDLIST pidl = nullptr;
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_File, nullptr, &pidl);

            Assert::AreNotEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should fail for null name.");
            Assert::IsNull(pidl, L"PIDL should be null when name is null.");
        }

        /// <summary>
        /// Test AllocateBigDriveItemIdExport with null output pointer (should fail).
        /// </summary>
        TEST_METHOD(AllocateBigDriveItemIdExport_NullOutParam)
        {
            BSTR name = ::SysAllocString(L"TestFile.txt");
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_File, name, nullptr);

            Assert::AreNotEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should fail for null output pointer.");
            ::SysFreeString(name);
        }

        /// <summary>
        /// Test GetBigDriveItemNameFromPidlExport with a null PIDL (should fail).
        /// </summary>
        TEST_METHOD(GetBigDriveItemNameFromPidlExport_NullPidl)
        {
            STRRET strret = {};
            HRESULT hr = GetBigDriveItemNameFromPidlExport(nullptr, &strret);
            Assert::AreNotEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should fail for null PIDL.");
        }

        /// <summary>
        /// Test GetBigDriveItemNameFromPidlExport with a malformed PIDL (should fail).
        /// </summary>
        TEST_METHOD(GetBigDriveItemNameFromPidlExport_MalformedPidl)
        {
            // Create a buffer that is too small to be a valid BIGDRIVE_ITEMID
            BYTE buffer[sizeof(USHORT) + sizeof(int)];
            *(USHORT*)buffer = static_cast<USHORT>(sizeof(buffer)); // cb
            *(int*)(buffer + sizeof(USHORT)) = static_cast<int>(BigDriveItemType_File); // nType
            // No name, no null terminator, not a valid PIDL

            STRRET strret = {};
            HRESULT hr = GetBigDriveItemNameFromPidlExport(reinterpret_cast<PCUIDLIST_RELATIVE>(buffer), &strret);
            Assert::AreNotEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should fail for malformed PIDL.");
        }

        /// <summary>
        /// Test GetBigDriveItemNameFromPidlExport with a valid PIDL but empty name.
        /// </summary>
        TEST_METHOD(GetBigDriveItemNameFromPidlExport_EmptyName)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR name = ::SysAllocString(L"");
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_File, name, &pidl);

            Assert::AreEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should succeed for empty name.");
            Assert::IsNotNull(pidl, L"PIDL should not be null for empty name.");

            if (pidl)
            {
                STRRET strret = {};
                hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
                Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for empty name.");
                Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
                Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null for empty name.");
                Assert::AreEqual(std::wstring(L""), std::wstring(strret.pOleStr), L"Extracted name should be empty.");
                ::CoTaskMemFree(strret.pOleStr);
                ::CoTaskMemFree(pidl);
            }
            ::SysFreeString(name);
        }
    };
}