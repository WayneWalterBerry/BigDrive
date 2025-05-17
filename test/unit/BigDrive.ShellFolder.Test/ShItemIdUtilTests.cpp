// <copyright file="ShItemIdUtilTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <CppUnitTest.h>
#include <comdef.h>

#include "ShItemIdUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    /// <summary>
    /// Unit tests for the ShItemIdUtil class, focusing on serialization and deserialization of SHITEMID structures.
    /// </summary>
    TEST_CLASS(ShItemIdUtilTests)
    {
    public:

        /// <summary>
        /// Tests that serializing a SHITEMID with readable ASCII bytes produces a BSTR with the same characters.
        /// </summary>
        TEST_METHOD(Serialize_SimpleReadable)
        {
            // Arrange: SHITEMID with abID = {'A', 'B', 'C'}
            BYTE abid[] = { 'A', 'B', 'C' };
            USHORT cb = static_cast<USHORT>(sizeof(USHORT) + sizeof(abid));
            SHITEMID* shitemid = (SHITEMID*)CoTaskMemAlloc(cb);
            shitemid->cb = cb;
            memcpy(shitemid->abID, abid, sizeof(abid));
            BSTR bstr = nullptr;

            // Act
            HRESULT hr = ShItemIdUtil::Serialize(shitemid, bstr);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(bstr != nullptr);
            Assert::AreEqual(3U, SysStringLen(bstr));
            Assert::AreEqual(0, wcsncmp(bstr, L"ABC", 3));

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests that serializing a SHITEMID with zero-length abID produces an empty BSTR.
        /// </summary>
        TEST_METHOD(Serialize_Empty)
        {
            // Arrange: SHITEMID with abID length 0
            USHORT cb = sizeof(USHORT);
            SHITEMID* shitemid = (SHITEMID*)CoTaskMemAlloc(cb);
            shitemid->cb = cb;
            BSTR bstr = nullptr;

            // Act
            HRESULT hr = ShItemIdUtil::Serialize(shitemid, bstr);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(bstr != nullptr);
            Assert::AreEqual(0U, SysStringLen(bstr));

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests that passing a null SHITEMID pointer to Serialize returns E_INVALIDARG.
        /// </summary>
        TEST_METHOD(Serialize_Null)
        {
            // Act
            BSTR bstr = nullptr;
            HRESULT hr = ShItemIdUtil::Serialize(nullptr, bstr);

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr);
        }

        /// <summary>
        /// Tests that deserializing a BSTR with readable ASCII characters produces a SHITEMID with matching abID bytes.
        /// </summary>
        TEST_METHOD(Deserialize_SimpleReadable)
        {
            // Arrange: BSTR with "XYZ"
            BSTR bstr = ::SysAllocString(L"XYZ");
            SHITEMID* shitemid = nullptr;

            // Act
            HRESULT hr = ShItemIdUtil::Deserialize(bstr, &shitemid);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(shitemid != nullptr);
            Assert::AreEqual((USHORT)5, shitemid->cb); // 3 + sizeof(USHORT)
            Assert::AreEqual((BYTE)'X', shitemid->abID[0]);
            Assert::AreEqual((BYTE)'Y', shitemid->abID[1]);
            Assert::AreEqual((BYTE)'Z', shitemid->abID[2]);

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests that deserializing an empty BSTR produces a SHITEMID with only the cb field.
        /// </summary>
        TEST_METHOD(Deserialize_Empty)
        {
            // Arrange: Empty BSTR
            BSTR bstr = ::SysAllocString(L"");
            SHITEMID* shitemid = nullptr;

            // Act
            HRESULT hr = ShItemIdUtil::Deserialize(bstr, &shitemid);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(shitemid != nullptr);
            Assert::AreEqual(sizeof(USHORT), (size_t)shitemid->cb);

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests that passing a null BSTR to Deserialize returns E_INVALIDARG.
        /// </summary>
        TEST_METHOD(Deserialize_Null)
        {
            // Act
            SHITEMID* shitemid = nullptr;
            HRESULT hr = ShItemIdUtil::Deserialize(nullptr, &shitemid);

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr);
        }

        /// <summary>
        /// Tests that passing a null SHITEMID** pointer to Deserialize returns E_INVALIDARG.
        /// </summary>
        TEST_METHOD(Deserialize_NullPointer)
        {
            // Arrange: BSTR with "A"
            BSTR bstr = ::SysAllocString(L"A");

            // Act
            HRESULT hr = ShItemIdUtil::Deserialize(bstr, nullptr);

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr);

            // Cleanup
            SysFreeString(bstr);
        }

        /// <summary>
        /// Tests that serializing and then deserializing a SHITEMID results in an equivalent structure.
        /// </summary>
        TEST_METHOD(RoundTrip_SerializeThenDeserialize)
        {
            // Arrange: SHITEMID with abID = {'1', '2', '3', '4'}
            BYTE abid[] = { '1', '2', '3', '4' };
            USHORT cb = static_cast<USHORT>(sizeof(USHORT) + sizeof(abid));
            SHITEMID* shitemid = (SHITEMID*)CoTaskMemAlloc(cb);
            shitemid->cb = cb;
            memcpy(shitemid->abID, abid, sizeof(abid));
            BSTR bstr = nullptr;
            SHITEMID* roundTrip = nullptr;

            // Act
            HRESULT hr1 = ShItemIdUtil::Serialize(shitemid, bstr);
            HRESULT hr2 = ShItemIdUtil::Deserialize(bstr, &roundTrip);

            // Assert
            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);
            Assert::IsTrue(roundTrip != nullptr);
            Assert::AreEqual(cb, roundTrip->cb);
            Assert::IsTrue(0 == memcmp(shitemid->abID, roundTrip->abID, sizeof(abid)));

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
            CoTaskMemFree(roundTrip);
        }

        /// <summary>
        /// Tests that deserializing and then serializing a BSTR results in an equivalent BSTR.
        /// </summary>
        TEST_METHOD(RoundTrip_DeserializeThenSerialize)
        {
            // Arrange: BSTR with "Test"
            BSTR bstr = ::SysAllocString(L"Test");
            SHITEMID* shitemid = nullptr;
            BSTR bstr2 = nullptr;

            // Act
            HRESULT hr1 = ShItemIdUtil::Deserialize(bstr, &shitemid);
            HRESULT hr2 = ShItemIdUtil::Serialize(shitemid, bstr2);

            // Assert
            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);
            Assert::IsTrue(bstr2 != nullptr);
            Assert::AreEqual(4U, SysStringLen(bstr2));
            Assert::AreEqual(0, wcsncmp(bstr, bstr2, 4));

            // Cleanup
            SysFreeString(bstr);
            SysFreeString(bstr2);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests that serializing and deserializing SHITEMID with Unicode (non-ASCII) bytes works as expected.
        /// </summary>
        TEST_METHOD(Serialize_UnicodeBytes)
        {
            // Arrange: SHITEMID with abID = {0xC3, 0xA9, 0xE2, 0x82, 0xAC} (UTF-8 for "é" and "€")
            BYTE abid[] = { 0xC3, 0xA9, 0xE2, 0x82, 0xAC };
            USHORT cb = static_cast<USHORT>(sizeof(USHORT) + sizeof(abid));
            SHITEMID* shitemid = (SHITEMID*)CoTaskMemAlloc(cb);
            shitemid->cb = cb;
            memcpy(shitemid->abID, abid, sizeof(abid));
            BSTR bstr = nullptr;

            // Act
            HRESULT hr = ShItemIdUtil::Serialize(shitemid, bstr);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(bstr != nullptr);
            Assert::AreEqual(5U, SysStringLen(bstr));
            // The BSTR will contain the bytes as wchar_t, not as Unicode codepoints
            Assert::AreEqual((wchar_t)0x00C3, bstr[0]);
            Assert::AreEqual((wchar_t)0x00A9, bstr[1]);
            Assert::AreEqual((wchar_t)0x00E2, bstr[2]);
            Assert::AreEqual((wchar_t)0x0082, bstr[3]);
            Assert::AreEqual((wchar_t)0x00AC, bstr[4]);

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests that deserializing a BSTR with Unicode (non-ASCII) characters produces the correct abID bytes.
        /// </summary>
        TEST_METHOD(Deserialize_UnicodeChars)
        {
            // Arrange: BSTR with L"\u03A9\u20AC" (Greek Omega and Euro sign)
            BSTR bstr = ::SysAllocString(L"\u03A9\u20AC");
            SHITEMID* shitemid = nullptr;

            // Act
            HRESULT hr = ShItemIdUtil::Deserialize(bstr, &shitemid);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(shitemid != nullptr);
            Assert::AreEqual((USHORT)4, shitemid->cb);
            // Only the low byte of each wchar_t is stored in abID
            Assert::AreEqual((BYTE)0xA9, shitemid->abID[0]); // 0x03A9 -> 0xA9
            Assert::AreEqual((BYTE)0xAC, shitemid->abID[1]); // 0x20AC -> 0xAC

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
        }

        /// <summary>
        /// Tests round-trip of Unicode bytes: serialize and then deserialize, and verify abID matches original bytes.
        /// </summary>
        TEST_METHOD(RoundTrip_UnicodeBytes)
        {
            // Arrange: SHITEMID with abID = {0xD0, 0x9F, 0xD1, 0x80, 0xD0, 0xB8, 0xD0, 0xB2, 0xD0, 0xB5, 0xD1, 0x82} ("Привет" in UTF-8 bytes)
            BYTE abid[] = { 0xD0, 0x9F, 0xD1, 0x80, 0xD0, 0xB8, 0xD0, 0xB2, 0xD0, 0xB5, 0xD1, 0x82 };
            USHORT cb = static_cast<USHORT>(sizeof(USHORT) + sizeof(abid));
            SHITEMID* shitemid = (SHITEMID*)CoTaskMemAlloc(cb);
            shitemid->cb = cb;
            memcpy(shitemid->abID, abid, sizeof(abid));
            BSTR bstr = nullptr;
            SHITEMID* roundTrip = nullptr;

            // Act
            HRESULT hr1 = ShItemIdUtil::Serialize(shitemid, bstr);
            HRESULT hr2 = ShItemIdUtil::Deserialize(bstr, &roundTrip);

            // Assert
            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);
            Assert::IsTrue(roundTrip != nullptr);
            Assert::AreEqual(cb, roundTrip->cb);
            Assert::IsTrue(0 == memcmp(shitemid->abID, roundTrip->abID, sizeof(abid)));

            // Cleanup
            SysFreeString(bstr);
            CoTaskMemFree(shitemid);
            CoTaskMemFree(roundTrip);
        }
    };
}