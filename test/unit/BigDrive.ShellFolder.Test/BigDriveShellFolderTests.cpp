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
    TEST_CLASS(BigDriveShellFolderTests)
    {
    public:

        /// <summary>
        /// Test AllocateBigDriveItemIdExport with valid file item.
        /// </summary>
        TEST_METHOD(AllocateBigDriveItemIdExport_File)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR name = ::SysAllocString(L"TestFile.txt");
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_File, name, &pidl);

            Assert::AreEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should succeed for file.");
            Assert::IsNotNull(pidl, L"PIDL should not be null for valid file.");

            if (pidl)
                ::CoTaskMemFree(pidl);
            ::SysFreeString(name);
        }

        /// <summary>
        /// Test AllocateBigDriveItemIdExport with valid folder item.
        /// </summary>
        TEST_METHOD(AllocateBigDriveItemIdExport_Folder)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR name = ::SysAllocString(L"TestFolder");
            HRESULT hr = AllocateBigDriveItemIdExport(BigDriveItemType_Folder, name, &pidl);

            Assert::AreEqual(S_OK, hr, L"AllocateBigDriveItemIdExport should succeed for folder.");
            Assert::IsNotNull(pidl, L"PIDL should not be null for valid folder.");

            if (pidl)
                ::CoTaskMemFree(pidl);
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
    };
}