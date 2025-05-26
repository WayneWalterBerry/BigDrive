// <copyright file="BigDriveEnumIDListTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
//   Unit tests for BigDriveEnumIDList via BigDriveEnumIDListImports.h.
//   These tests validate construction, enumeration, reference counting, and mutation
//   of the BigDriveEnumIDList class through its C API.
// </summary>

#include "pch.h"
#include <windows.h>
#include "CppUnitTest.h"
#include "..\..\..\src\IShellFolder\Exports\BigDriveEnumIDListImports.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    /// <summary>
    /// Unit tests for the BigDriveEnumIDList class via its import API.
    /// </summary>
    TEST_CLASS(BigDriveEnumIDListTests)
    {
    public:

        /// <summary>
        /// Tests that a BigDriveEnumIDList can be constructed and destroyed without error.
        /// </summary>
        TEST_METHOD(CreateAndDestroy)
        {
            BigDriveEnumIDList* pEnum = CreateBigDriveEnumIDList();
            Assert::IsNotNull(pEnum, L"Failed to create BigDriveEnumIDList.");
            DestroyBigDriveEnumIDList(pEnum);
        }

        /// <summary>
        /// Tests reference counting using AddRef and Release.
        /// </summary>
        TEST_METHOD(ReferenceCounting)
        {
            BigDriveEnumIDList* pEnum = CreateBigDriveEnumIDList();
            Assert::IsNotNull(pEnum, L"Failed to create BigDriveEnumIDList.");

            ULONG ref1 = BigDriveEnumIDList_AddRef(pEnum);
            Assert::IsTrue(ref1 > 1, L"AddRef did not increment reference count.");

            ULONG ref2 = BigDriveEnumIDList_Release(pEnum);
            Assert::IsTrue(ref2 >= 1, L"Release did not decrement reference count properly.");

            // Final release to delete object
            BigDriveEnumIDList_Release(pEnum);
        }

        /// <summary>
        /// Tests QueryInterface returns E_NOINTERFACE for unsupported interfaces and S_OK for IUnknown.
        /// </summary>
        TEST_METHOD(QueryInterface)
        {
            BigDriveEnumIDList* pEnum = CreateBigDriveEnumIDList();
            Assert::IsNotNull(pEnum, L"Failed to create BigDriveEnumIDList.");

            void* pUnknown = nullptr;
            HRESULT hr = BigDriveEnumIDList_QueryInterface(pEnum, IID_IUnknown, &pUnknown);
            Assert::AreEqual(S_OK, hr, L"QueryInterface for IUnknown failed.");
            Assert::IsNotNull(pUnknown, L"QueryInterface did not return a valid pointer for IUnknown.");

            // Query for a random GUID should fail
            GUID bogus = { 0xDEADBEEF, 0xBEEF, 0xDEAD, {0,0,0,0,0,0,0,0} };
            void* pBogus = nullptr;
            hr = BigDriveEnumIDList_QueryInterface(pEnum, bogus, &pBogus);
            Assert::AreEqual(E_NOINTERFACE, hr, L"QueryInterface for bogus IID should fail.");

            BigDriveEnumIDList_Release(pEnum);
        }

        /// <summary>
        /// Tests enumeration using Next, Skip, and Reset.
        /// </summary>
        TEST_METHOD(EnumerateAndReset)
        {
            // Create a fake PIDL array (normally these would be valid, but for test, just allocate memory)
            LPITEMIDLIST pidls[2] = { (LPITEMIDLIST)CoTaskMemAlloc(32), (LPITEMIDLIST)CoTaskMemAlloc(32) };
            Assert::IsNotNull(pidls[0], L"Failed to allocate PIDL 0.");
            Assert::IsNotNull(pidls[1], L"Failed to allocate PIDL 1.");

            BigDriveEnumIDList* pEnum = CreateBigDriveEnumIDListWithItems(pidls, 2);
            Assert::IsNotNull(pEnum, L"Failed to create BigDriveEnumIDList with items.");

            LPITEMIDLIST fetched[2] = {};
            ULONG fetchedCount = 0;
            HRESULT hr = BigDriveEnumIDList_Next(pEnum, 2, fetched, &fetchedCount);
            Assert::IsTrue(hr == S_OK || hr == S_FALSE, L"Next did not return S_OK or S_FALSE.");
            Assert::IsTrue(fetchedCount <= 2, L"Fetched more items than requested.");

            // Skip should return S_FALSE if at end
            hr = BigDriveEnumIDList_Skip(pEnum, 1);
            Assert::IsTrue(hr == S_OK || hr == S_FALSE, L"Skip did not return S_OK or S_FALSE.");

            // Reset and enumerate again
            hr = BigDriveEnumIDList_Reset(pEnum);
            Assert::AreEqual(S_OK, hr, L"Reset failed.");

            // Clean up
            BigDriveEnumIDList_Release(pEnum);
            CoTaskMemFree(pidls[0]);
            CoTaskMemFree(pidls[1]);
        }

        /// <summary>
        /// Tests Clone creates a new enumerator with the same state.
        /// </summary>
        TEST_METHOD(CloneEnumerator)
        {
            LPITEMIDLIST pidls[1] = { (LPITEMIDLIST)CoTaskMemAlloc(32) };
            Assert::IsNotNull(pidls[0], L"Failed to allocate PIDL.");

            BigDriveEnumIDList* pEnum = CreateBigDriveEnumIDListWithItems(pidls, 1);
            Assert::IsNotNull(pEnum, L"Failed to create BigDriveEnumIDList with items.");

            IEnumIDList* pCloned = nullptr;
            HRESULT hr = BigDriveEnumIDList_Clone(pEnum, &pCloned);
            Assert::AreEqual(S_OK, hr, L"Clone failed.");
            Assert::IsNotNull(pCloned, L"Cloned enumerator is null.");

            // Release both
            pCloned->Release();
            BigDriveEnumIDList_Release(pEnum);
            CoTaskMemFree(pidls[0]);
        }

        /// <summary>
        /// Tests Add method to ensure a PIDL can be added to the enumerator.
        /// </summary>
        TEST_METHOD(AddPidl)
        {
            BigDriveEnumIDList* pEnum = CreateBigDriveEnumIDList();
            Assert::IsNotNull(pEnum, L"Failed to create BigDriveEnumIDList.");

            LPITEMIDLIST pidl = (LPITEMIDLIST)CoTaskMemAlloc(32);
            Assert::IsNotNull(pidl, L"Failed to allocate PIDL.");

            HRESULT hr = BigDriveEnumIDList_Add(pEnum, pidl);
            Assert::AreEqual(S_OK, hr, L"Add did not return S_OK.");

            BigDriveEnumIDList_Release(pEnum);
            CoTaskMemFree(pidl);
        }
    };
}