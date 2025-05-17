// <copyright file="ItemIdListTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <CppUnitTest.h>
#include <comdef.h>
#include "ItemIdList.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{

    /// <summary>
    /// Helper function to create a SHITEMID list (LPITEMIDLIST) from an array of abID byte arrays.
    /// Each abID array represents the abID portion of a SHITEMID structure.
    /// </summary>
    /// <param name="abids">Array of pointers to abID byte arrays.</param>
    /// <param name="abidLens">Array of lengths for each abID array.</param>
    /// <param name="itemCount">Number of items in the list.</param>
    /// <returns>Pointer to a newly allocated ITEMIDLIST. Caller must free with CoTaskMemFree.</returns>
    LPITEMIDLIST CreateItemIdList(const BYTE** abids, const size_t* abidLens, size_t itemCount)
    {
        // Calculate total size: sum of all SHITEMID sizes + 2 bytes for terminator
        size_t total = 0;
        for (size_t i = 0; i < itemCount; ++i)
            total += sizeof(USHORT) + abidLens[i];
        total += sizeof(USHORT); // terminator

        BYTE* buffer = (BYTE*)CoTaskMemAlloc(total);
        BYTE* p = buffer;
        for (size_t i = 0; i < itemCount; ++i)
        {
            USHORT cb = static_cast<USHORT>(sizeof(USHORT) + abidLens[i]);
            *(USHORT*)p = cb;
            if (abidLens[i] > 0)
                memcpy(p + sizeof(USHORT), abids[i], abidLens[i]);
            p += cb;
        }
        // Add terminator
        *(USHORT*)p = 0;
        return reinterpret_cast<LPITEMIDLIST>(buffer);
    }

    /// <summary>
    /// Unit tests for the ItemIdList class, focusing on serialization and deserialization of ITEMIDLIST structures.
    /// </summary>
    TEST_CLASS(ItemIdListTests)
    {
    public:

        /// <summary>
        /// Tests that serializing an empty ITEMIDLIST (only terminator) returns an empty string.
        /// </summary>
        TEST_METHOD(SerializeList_EmptyList_ReturnsEmptyString)
        {
            // Arrange: Only terminator
            LPITEMIDLIST pidl = (LPITEMIDLIST)CoTaskMemAlloc(sizeof(USHORT));
            *(USHORT*)pidl = 0;
            ItemIdList list(pidl);
            BSTR result = nullptr;

            // Act
            HRESULT hr = list.SerializeList(result);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(0U, SysStringLen(result));
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that serializing a single-item ITEMIDLIST produces the correct hex string.
        /// </summary>
        TEST_METHOD(SerializeList_SingleItem)
        {
            // Arrange: abID = {0x12, 0xAB}
            BYTE abid1[] = { 0x12, 0xAB };
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { sizeof(abid1) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            ItemIdList list(pidl);
            BSTR result = nullptr;

            // Act
            HRESULT hr = list.SerializeList(result);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(L"12AB", result);
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that serializing a multi-item ITEMIDLIST produces the correct hex string with '/' separators.
        /// </summary>
        TEST_METHOD(SerializeList_MultipleItems)
        {
            // Arrange: abID1 = {0x01}, abID2 = {0xFF, 0x00}
            BYTE abid1[] = { 0x01 };
            BYTE abid2[] = { 0xFF, 0x00 };
            const BYTE* abids[] = { abid1, abid2 };
            size_t abidLens[] = { sizeof(abid1), sizeof(abid2) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 2);
            ItemIdList list(pidl);
            BSTR result = nullptr;

            // Act
            HRESULT hr = list.SerializeList(result);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(L"01/FF00", result);
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that serializing an ITEMIDLIST with a zero-length abID returns an empty string.
        /// </summary>
        TEST_METHOD(SerializeList_ZeroLengthabID)
        {
            // Arrange: abID = {}
            BYTE abid1[] = { 0 }; // Dynamically sized array with at least one element
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { 0 };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            ItemIdList list(pidl);
            BSTR result = nullptr;

            // Act
            HRESULT hr = list.SerializeList(result);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(L"", result);
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that passing a null BSTR pointer to SerializeList returns E_INVALIDARG.
        /// </summary>
        TEST_METHOD(SerializeList_NullBstrPointer)
        {
            // Arrange: abID = {0x01}
            BYTE abid1[] = { 0x01 };
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { sizeof(abid1) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            ItemIdList list(pidl);

            // Act
            HRESULT hr = list.SerializeList(*reinterpret_cast<BSTR*>(nullptr));

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr);
            CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that deserializing an empty string produces a valid ITEMIDLIST with only a terminator.
        /// </summary>
        TEST_METHOD(DeserializeList_EmptyString_ReturnsTerminator)
        {
            // Arrange
            LPITEMIDLIST pidl = nullptr;

            // Act
            HRESULT hr = ItemIdList::DeserializeList(SysAllocString(L""), &pidl);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);
            Assert::AreEqual((USHORT)0, *(USHORT*)pidl);
            ::CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that deserializing a single-item hex string produces the correct ITEMIDLIST structure.
        /// </summary>
        TEST_METHOD(DeserializeList_SingleItem)
        {
            // Arrange
            LPITEMIDLIST pidl = nullptr;

            // Act
            HRESULT hr = ItemIdList::DeserializeList(SysAllocString(L"12AB"), &pidl);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);
            // Check first SHITEMID
            USHORT cb = *(USHORT*)((BYTE*)pidl);
            Assert::AreEqual((USHORT)4, cb); // 2 bytes for USHORT + 2 bytes abID
            BYTE* abID = ((BYTE*)pidl) + sizeof(USHORT);
            Assert::AreEqual((BYTE)0x12, abID[0]);
            Assert::AreEqual((BYTE)0xAB, abID[1]);
            // Check terminator
            USHORT term = *(USHORT*)((BYTE*)pidl + cb);
            Assert::AreEqual((USHORT)0, term);
            ::CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that deserializing a multi-item hex string produces the correct ITEMIDLIST structure.
        /// </summary>
        TEST_METHOD(DeserializeList_MultipleItems)
        {
            // Arrange
            LPITEMIDLIST pidl = nullptr;

            // Act
            HRESULT hr = ItemIdList::DeserializeList(SysAllocString(L"01/FF00"), &pidl);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);
            // First item
            USHORT cb1 = *(USHORT*)((BYTE*)pidl);
            Assert::AreEqual((USHORT)3, cb1); // 2 bytes for USHORT + 1 byte abID
            BYTE* abID1 = ((BYTE*)pidl) + sizeof(USHORT);
            Assert::AreEqual((BYTE)0x01, abID1[0]);
            // Second item
            BYTE* next = ((BYTE*)pidl) + cb1;
            USHORT cb2 = *(USHORT*)next;
            Assert::AreEqual((USHORT)4, cb2); // 2 bytes for USHORT + 2 bytes abID
            BYTE* abID2 = next + sizeof(USHORT);
            Assert::AreEqual((BYTE)0xFF, abID2[0]);
            Assert::AreEqual((BYTE)0x00, abID2[1]);
            // Terminator
            USHORT term = *(USHORT*)(next + cb2);
            Assert::AreEqual((USHORT)0, term);
            ::CoTaskMemFree(pidl);
        }

        /// <summary>
        /// Tests that deserializing a string with invalid hex characters returns E_INVALIDARG and does not allocate memory.
        /// </summary>
        TEST_METHOD(DeserializeList_InvalidHex_ReturnsError)
        {
            // Arrange
            LPITEMIDLIST pidl = nullptr;

            // Act
            HRESULT hr = ItemIdList::DeserializeList(SysAllocString(L"ZZ"), &pidl);

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr);
            Assert::IsTrue(pidl == nullptr);
        }

        /// <summary>
        /// Tests that passing a null pointer for the output LPITEMIDLIST returns E_POINTER.
        /// </summary>
        TEST_METHOD(DeserializeList_NullPointer_ReturnsPointerError)
        {
            // Act
            HRESULT hr = ItemIdList::DeserializeList(SysAllocString(L"12AB"), nullptr);

            // Assert
            Assert::AreEqual(E_POINTER, hr);
        }

        /// <summary>
        /// Tests that serializing and then deserializing a single-item ITEMIDLIST results in an equivalent structure.
        /// </summary>
        TEST_METHOD(SerializeThenDeserialize_RoundTrip_SingleItem)
        {
            // Arrange
            BYTE abid1[] = { 0x12, 0xAB };
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { sizeof(abid1) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            ItemIdList list(pidl);
            BSTR serialized = nullptr;
            LPITEMIDLIST roundTrip = nullptr;

            // Act
            HRESULT hr1 = list.SerializeList(serialized);
            HRESULT hr2 = ItemIdList::DeserializeList(serialized, &roundTrip);

            // Assert
            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);
            // Check that the round-tripped abID matches
            USHORT cb = *(USHORT*)((BYTE*)roundTrip);
            Assert::AreEqual((USHORT)4, cb);
            BYTE* abID = ((BYTE*)roundTrip) + sizeof(USHORT);
            Assert::AreEqual((BYTE)0x12, abID[0]);
            Assert::AreEqual((BYTE)0xAB, abID[1]);
            ::SysFreeString(serialized);
            ::CoTaskMemFree(pidl);
            ::CoTaskMemFree(roundTrip);
        }

        /// <summary>
        /// Tests that serializing and then deserializing a multi-item ITEMIDLIST results in an equivalent structure.
        /// </summary>
        TEST_METHOD(SerializeThenDeserialize_RoundTrip_MultiItem)
        {
            // Arrange
            BYTE abid1[] = { 0x01 };
            BYTE abid2[] = { 0xFF, 0x00 };
            BYTE abid3[] = { 0xAA, 0xBB, 0xCC };
            const BYTE* abids[] = { abid1, abid2, abid3 };
            size_t abidLens[] = { sizeof(abid1), sizeof(abid2), sizeof(abid3) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 3);
            ItemIdList list(pidl);
            BSTR serialized = nullptr;
            LPITEMIDLIST roundTrip = nullptr;

            // Act
            HRESULT hr1 = list.SerializeList(serialized);
            HRESULT hr2 = ItemIdList::DeserializeList(serialized, &roundTrip);

            // Assert
            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);

            // Check all abIDs
            BYTE* cur = (BYTE*)roundTrip;
            USHORT cb1 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)3, cb1);
            Assert::AreEqual((BYTE)0x01, cur[2]);
            cur += cb1;
            USHORT cb2 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)4, cb2);
            Assert::AreEqual((BYTE)0xFF, cur[2]);
            Assert::AreEqual((BYTE)0x00, cur[3]);
            cur += cb2;
            USHORT cb3 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)5, cb3);
            Assert::AreEqual((BYTE)0xAA, cur[2]);
            Assert::AreEqual((BYTE)0xBB, cur[3]);
            Assert::AreEqual((BYTE)0xCC, cur[4]);
            cur += cb3;
            USHORT term = *(USHORT*)cur;
            Assert::AreEqual((USHORT)0, term);

            ::SysFreeString(serialized);
            ::CoTaskMemFree(pidl);
            ::CoTaskMemFree(roundTrip);
        }

        /// <summary>
        /// Tests that serializing and then deserializing an ITEMIDLIST with 8 items (with increasing abID lengths) results in an equivalent structure.
        /// </summary>
        TEST_METHOD(SerializeThenDeserialize_RoundTrip_8Items)
        {
            // Arrange: 8 items, each with 1-8 bytes
            BYTE abid1[] = { 0x01 };
            BYTE abid2[] = { 0x02, 0x03 };
            BYTE abid3[] = { 0x04, 0x05, 0x06 };
            BYTE abid4[] = { 0x07, 0x08, 0x09, 0x0A };
            BYTE abid5[] = { 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
            BYTE abid6[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };
            BYTE abid7[] = { 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C };
            BYTE abid8[] = { 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24 };
            const BYTE* abids[] = { abid1, abid2, abid3, abid4, abid5, abid6, abid7, abid8 };
            size_t abidLens[] = { sizeof(abid1), sizeof(abid2), sizeof(abid3), sizeof(abid4), sizeof(abid5), sizeof(abid6), sizeof(abid7), sizeof(abid8) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 8);
            ItemIdList list(pidl);
            BSTR serialized = nullptr;
            LPITEMIDLIST roundTrip = nullptr;

            // Act
            HRESULT hr1 = list.SerializeList(serialized);
            HRESULT hr2 = ItemIdList::DeserializeList(serialized, &roundTrip);

            // Assert
            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);

            // Check all abIDs
            BYTE* cur = (BYTE*)roundTrip;
            USHORT cbs[8];
            size_t offset = 0;
            for (int i = 0; i < 8; ++i)
            {
                cbs[i] = *(USHORT*)(cur + offset);
                Assert::AreEqual((USHORT)(sizeof(USHORT) + abidLens[i]), cbs[i]);
                for (size_t j = 0; j < abidLens[i]; ++j)
                {
                    Assert::AreEqual(abids[i][j], *(cur + offset + sizeof(USHORT) + j));
                }
                offset += cbs[i];
            }
            USHORT term = *(USHORT*)(cur + offset);
            Assert::AreEqual((USHORT)0, term);

            ::SysFreeString(serialized);
            ::CoTaskMemFree(pidl);
            ::CoTaskMemFree(roundTrip);
        }

        /// <summary>
        /// Tests that DeserializeList correctly handles input with more than 16 items,
        /// which triggers heap allocation for the abidLens array (stack fallback).
        /// Verifies that the resulting ITEMIDLIST is valid and matches the input.
        /// </summary>
        TEST_METHOD(DeserializeList_MoreThan16Items_HeapAllocFallback)
        {
            // Arrange: Build a hex string for 20 items, each abID is "01"
            std::wstring input;
            for (int i = 0; i < 20; ++i)
            {
                if (i > 0) input += L'/';
                input += L"01";
            }

            LPITEMIDLIST pidl = nullptr;

            // Act
            HRESULT hr = ItemIdList::DeserializeList(SysAllocString(input.c_str()), &pidl);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);

            // Walk the resulting ITEMIDLIST and verify each abID is 0x01
            BYTE* cur = (BYTE*)pidl;
            for (int i = 0; i < 20; ++i)
            {
                USHORT cb = *(USHORT*)cur;
                Assert::AreEqual((USHORT)3, cb); // 2 bytes for USHORT + 1 byte abID
                Assert::AreEqual((BYTE)0x01, cur[2]);
                cur += cb;
            }
            // Check terminator
            USHORT term = *(USHORT*)cur;
            Assert::AreEqual((USHORT)0, term);

            ::CoTaskMemFree(pidl);
        }
    };
}