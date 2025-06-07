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
#include "..\..\..\src\IShellFolder\ILExtensions.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
	/// <summary>
	/// Unit tests for BigDriveShellFolder PIDL allocation and name extraction.
	/// </summary>
	TEST_CLASS(BigDriveShellFolderTests)
	{
	public:

		BigDriveShellFolderTests()
		{
			::EnableMemoryLeakChecks();
		}

		/// <summary>
		/// Test AllocBigDrivePidlExport with valid file item and verify the name using GetBigDriveItemNameFromPidlExport.
		/// </summary>
		TEST_METHOD(AllocBigDrivePidl_File)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR name = ::SysAllocString(L"TestFile.txt");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, name, &pidl);

			Assert::AreEqual(S_OK, hr, L"AllocBigDrivePidlExport should succeed for file.");
			Assert::IsNotNull(pidl, L"PIDL should not be null for valid file.");
			Assert::AreEqual(::ILGetCount(pidl), 1U, L"PIDL should contain exactly one item.");

			STRRET strret = {};
			hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
			Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for file PIDL.");
			Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
			Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
			Assert::AreEqual(L"TestFile.txt", strret.pOleStr, L"Extracted file name does not match.");

			BSTR bstrPath;
			hr = GetPathForProvidersExport(pidl, bstrPath);
			Assert::IsTrue(bstrPath != nullptr, L"Extracted path should not be null.");
			Assert::AreEqual(S_OK, hr, L"GetPathExport() should succeed for folder PIDL.");
			Assert::AreEqual(L"\\TestFile.txt", bstrPath, L"Extracted path does not match.");

			::CoTaskMemFree(strret.pOleStr);
			::CoTaskMemFree(pidl);
			::SysFreeString(name);
		}

		/// <summary>
		/// Test AllocBigDrivePidlExport with valid folder item and verify the name using GetBigDriveItemNameFromPidlExport.
		/// </summary>
		TEST_METHOD(AllocBigDrivePidl_Folder)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR name = ::SysAllocString(L"TestFolder");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_Folder, name, &pidl);

			Assert::AreEqual(S_OK, hr, L"AllocBigDrivePidlExport should succeed for folder.");
			Assert::IsNotNull(pidl, L"PIDL should not be null for valid folder.");
			Assert::AreEqual(1U, ::ILGetCount(pidl), L"PIDL should contain exactly one item.");

			STRRET strret = {};
			hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
			Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for folder PIDL.");
			Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
			Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
			Assert::AreEqual(L"TestFolder", strret.pOleStr, L"Extracted folder name does not match.");

			::CoTaskMemFree(strret.pOleStr);
			::CoTaskMemFree(pidl);
			::SysFreeString(name);
		}

		TEST_METHOD(AllocBigDrivePidl_FolderFile)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR name = ::SysAllocString(L"TestFolder\\File.txt");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, name, &pidl);

			Assert::AreEqual(S_OK, hr, L"AllocBigDrivePidlExport should succeed for a valid path.");
			Assert::IsNotNull(pidl, L"PIDL should not be null for valid path.");

			STRRET strret = {};
			hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
			Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for folder PIDL.");
			Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
			Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
			Assert::AreEqual(L"File.txt", strret.pOleStr, L"Extracted file name does not match.");

			BSTR bstrPath;
			hr = GetPathForProvidersExport(pidl, bstrPath);
			Assert::IsTrue(bstrPath != nullptr, L"Extracted path should not be null.");
			Assert::AreEqual(S_OK, hr, L"GetPathExport() should succeed for folder PIDL.");
			Assert::AreEqual(L"\\TestFolder\\File.txt", bstrPath, L"Extracted path does not match.");

			Assert::AreEqual(2U, ::ILGetCount(pidl), L"PIDL should contain exactly two items.");

			const SHITEMID* pShItemId1;
			hr = ::ILGetItemAt(pidl, 0, &pShItemId1);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId1 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId1);
			Assert::AreEqual((UINT)BigDriveItemType_Folder, pBigDriveItemId1->uType, L"First item should be a folder.");
			Assert::AreEqual(L"TestFolder", pBigDriveItemId1->szName, L"First item name should match.");

			const SHITEMID* pShItemId2;
			hr = ::ILGetItemAt(pidl, 1, &pShItemId2);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId2 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId2);
			Assert::AreEqual((UINT)BigDriveItemType_File, pBigDriveItemId2->uType, L"Second item should be a file.");
			Assert::AreEqual(L"File.txt", pBigDriveItemId2->szName, L"Second item name should match.");

			::SysFreeString(bstrPath);
			::CoTaskMemFree(strret.pOleStr);
			::CoTaskMemFree(pidl);
			::SysFreeString(name);
		}

		TEST_METHOD(AllocBigDrivePidl_RootFolderFile)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR name = ::SysAllocString(L"\\TestFolder\\File.txt");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, name, &pidl);

			Assert::AreEqual(S_OK, hr, L"AllocBigDrivePidlExport should succeed for a valid path.");
			Assert::IsNotNull(pidl, L"PIDL should not be null for valid path.");
			Assert::AreEqual(2U, ::ILGetCount(pidl), L"PIDL should contain exactly two items.");

			const SHITEMID* pShItemId1;
			hr = ::ILGetItemAt(pidl, 0, &pShItemId1);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId1 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId1);
			Assert::AreEqual((UINT)BigDriveItemType_Folder, pBigDriveItemId1->uType, L"First item should be a folder.");
			Assert::AreEqual(L"TestFolder", pBigDriveItemId1->szName, L"First item name should match.");

			const SHITEMID* pShItemId2;
			hr = ::ILGetItemAt(pidl, 1, &pShItemId2);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId2 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId2);
			Assert::AreEqual((UINT)BigDriveItemType_File, pBigDriveItemId2->uType, L"Second item should be a file.");
			Assert::AreEqual(L"File.txt", pBigDriveItemId2->szName, L"Second item name should match.");

			STRRET strret = {};
			hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
			Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for folder PIDL.");
			Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
			Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
			Assert::AreEqual(L"File.txt", strret.pOleStr, L"Extracted file name does not match.");

			BSTR bstrPath;
			hr = GetPathForProvidersExport(pidl, bstrPath);
			Assert::IsTrue(bstrPath != nullptr, L"Extracted path should not be null.");
			Assert::AreEqual(S_OK, hr, L"GetPathExport() should succeed for folder PIDL.");
			Assert::AreEqual(L"\\TestFolder\\File.txt", bstrPath, L"Extracted path does not match.");

			::SysFreeString(bstrPath);
			::CoTaskMemFree(strret.pOleStr);
			::CoTaskMemFree(pidl);
			::SysFreeString(name);
		}

		TEST_METHOD(AllocBigDrivePidl_FolderFolder)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR name = ::SysAllocString(L"TestFolder\\SubFolder");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_Folder, name, &pidl);

			Assert::AreEqual(S_OK, hr, L"AllocBigDrivePidlExport should succeed for a valid path.");
			Assert::IsNotNull(pidl, L"PIDL should not be null for valid path.");
			Assert::AreEqual(2U, ::ILGetCount(pidl), L"PIDL should contain exactly two items.");

			STRRET strret = {};
			hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
			Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for folder PIDL.");
			Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
			Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
			Assert::AreEqual(L"SubFolder", strret.pOleStr, L"Extracted folder name does not match.");

			BSTR bstrPath;
			hr = GetPathForProvidersExport(pidl, bstrPath);
			Assert::IsTrue(bstrPath != nullptr, L"Extracted path should not be null.");
			Assert::AreEqual(S_OK, hr, L"GetPathExport() should succeed for folder PIDL.");
			Assert::AreEqual(L"\\TestFolder\\SubFolder", bstrPath, L"Extracted path does not match.");

			const SHITEMID* pShItemId1;
			hr = ::ILGetItemAt(pidl, 0, &pShItemId1);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId1 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId1);
			Assert::AreEqual((UINT)BigDriveItemType_Folder, pBigDriveItemId1->uType, L"First item should be a folder.");
			Assert::AreEqual(L"TestFolder", pBigDriveItemId1->szName, L"First item name should match.");

			const SHITEMID* pShItemId2;
			hr = ::ILGetItemAt(pidl, 1, &pShItemId2);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId2 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId2);
			Assert::AreEqual((UINT)BigDriveItemType_Folder, pBigDriveItemId2->uType, L"Second item should be a file.");
			Assert::AreEqual(L"SubFolder", pBigDriveItemId2->szName, L"Second item name should match.");

			::SysFreeString(bstrPath);
			::CoTaskMemFree(strret.pOleStr);
			::CoTaskMemFree(pidl);
			::SysFreeString(name);
		}

		TEST_METHOD(AllocBigDrivePidl_FolderFolderFile)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR name = ::SysAllocString(L"TestFolder\\SubFolder\\TestFile.txt");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, name, &pidl);

			Assert::AreEqual(S_OK, hr, L"AllocBigDrivePidlExport should succeed for a valid path.");
			Assert::IsNotNull(pidl, L"PIDL should not be null for valid path.");

			STRRET strret = {};
			hr = GetBigDriveItemNameFromPidlExport(pidl, &strret);
			Assert::AreEqual(S_OK, hr, L"GetBigDriveItemNameFromPidlExport should succeed for folder PIDL.");
			Assert::AreEqual((UINT)STRRET_WSTR, (UINT)strret.uType, L"STRRET type should be STRRET_WSTR.");
			Assert::IsTrue(strret.pOleStr != nullptr, L"Extracted name should not be null.");
			Assert::AreEqual(L"TestFile.txt", strret.pOleStr, L"Extracted folder name does not match.");

			BSTR bstrPath;
			hr = GetPathForProvidersExport(pidl, bstrPath);
			Assert::IsTrue(bstrPath != nullptr, L"Extracted path should not be null.");
			Assert::AreEqual(S_OK, hr, L"GetPathExport() should succeed for folder PIDL.");
			Assert::AreEqual(L"\\TestFolder\\SubFolder\\TestFile.txt", bstrPath, L"Extracted path does not match.");

			Assert::AreEqual(3U, ::ILGetCount(pidl), L"PIDL should contain exactly three items.");

			const SHITEMID* pShItemId1;
			hr = ::ILGetItemAt(pidl, 0, &pShItemId1);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId1 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId1);
			Assert::AreEqual((UINT)BigDriveItemType_Folder, pBigDriveItemId1->uType, L"First item should be a folder.");
			Assert::AreEqual(L"TestFolder", pBigDriveItemId1->szName, L"First item name should match.");

			const SHITEMID* pShItemId2;
			hr = ::ILGetItemAt(pidl, 1, &pShItemId2);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId2 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId2);
			Assert::AreEqual((UINT)BigDriveItemType_Folder, pBigDriveItemId2->uType, L"Second item should be a folder.");
			Assert::AreEqual(L"SubFolder", pBigDriveItemId2->szName, L"Second item name should match.");

			const SHITEMID* pShItemId3;
			hr = ::ILGetItemAt(pidl, 2, &pShItemId3);
			Assert::AreEqual(S_OK, hr, L"ILGetItemAt() should succeed");
			const BIGDRIVE_ITEMID* pBigDriveItemId3 = reinterpret_cast<const BIGDRIVE_ITEMID*>(pShItemId3);
			Assert::AreEqual((UINT)BigDriveItemType_File, pBigDriveItemId3->uType, L"Third item should be a file.");
			Assert::AreEqual(L"TestFile.txt", pBigDriveItemId3->szName, L"Third item name should match.");

			::SysFreeString(bstrPath);
			::CoTaskMemFree(strret.pOleStr);
			::CoTaskMemFree(pidl);
			::SysFreeString(name);
		}

		/// <summary>
		/// Test AllocBigDrivePidlExport with null name (should fail).
		/// </summary>
		TEST_METHOD(AllocBigDrivePidlExport_NullName)
		{
			LPITEMIDLIST pidl = nullptr;
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, nullptr, &pidl);

			Assert::AreNotEqual(S_OK, hr, L"AllocBigDrivePidlExport should fail for null name.");
			Assert::IsNull(pidl, L"PIDL should be null when name is null.");
		}

		/// <summary>
		/// Test AllocBigDrivePidlExport with null output pointer (should fail).
		/// </summary>
		TEST_METHOD(AllocBigDrivePidlExport_NullOutParam)
		{
			BSTR name = ::SysAllocString(L"TestFile.txt");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, name, nullptr);

			Assert::AreNotEqual(S_OK, hr, L"AllocBigDrivePidlExport should fail for null output pointer.");
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
		/// Test GetBigDriveItemNameFromPidlExport with a valid PIDL but empty name.
		/// </summary>
		TEST_METHOD(GetBigDriveItemNameFromPidlExport_EmptyName)
		{
			LPITEMIDLIST pidl = nullptr;
			BSTR bstrPath = ::SysAllocString(L"");
			HRESULT hr = AllocBigDrivePidlExport(BigDriveItemType_File, bstrPath, &pidl);

			Assert::AreEqual(E_INVALIDARG, hr, L"AllocBigDrivePidlExport should not succeed for empty name.");

			::SysFreeString(bstrPath);
		}
	};
}