// <copyright file="ILExtensions.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <CppUnitTest.h>
#include <comdef.h>

#include "ILExtensions.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    /// <summary>
    /// Creates a SHITEMID list (LPITEMIDLIST) from an array of abID byte arrays.
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

    TEST_CLASS(ILExtensions)
    {
    public:

        TEST_METHOD(ILSerialize_EmptyList_ReturnsEmptyString)
        {
            LPITEMIDLIST pidl = (LPITEMIDLIST)CoTaskMemAlloc(sizeof(USHORT));
            *(USHORT*)pidl = 0;
            BSTR result = nullptr;

            HRESULT hr = ILSerialize(pidl, result);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(0U, SysStringLen(result));
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        TEST_METHOD(ILSerialize_SingleItem)
        {
            BYTE abid1[] = { 'A', 'B' };
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { sizeof(abid1) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            BSTR result = nullptr;

            HRESULT hr = ILSerialize(pidl, result);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(2U, SysStringLen(result));
            Assert::AreEqual(0, wcsncmp(result, L"AB", 2));
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        TEST_METHOD(ILSerialize_MultipleItems)
        {
            BYTE abid1[] = { 'X' };
            BYTE abid2[] = { 'Y', 'Z' };
            const BYTE* abids[] = { abid1, abid2 };
            size_t abidLens[] = { sizeof(abid1), sizeof(abid2) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 2);
            BSTR result = nullptr;

            HRESULT hr = ILSerialize(pidl, result);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(4U, SysStringLen(result));
            Assert::AreEqual(0, wcsncmp(result, L"X/YZ", 4));
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        TEST_METHOD(ILSerialize_ZeroLengthabID)
        {
            BYTE abid1[] = { 0 };
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { 0 };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            BSTR result = nullptr;

            HRESULT hr = ILSerialize(pidl, result);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(result != nullptr);
            Assert::AreEqual(0U, SysStringLen(result));
            SysFreeString(result);
            CoTaskMemFree(pidl);
        }

        TEST_METHOD(ILDeserialize_EmptyString_ReturnsTerminator)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR bstr = ::SysAllocString(L"");

            HRESULT hr = ILDeserialize(bstr, &pidl);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);
            Assert::AreEqual((USHORT)0, *(USHORT*)pidl);
            ::CoTaskMemFree(pidl);
            SysFreeString(bstr);
        }

        TEST_METHOD(ILDeserialize_SingleItem)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR bstr = ::SysAllocString(L"AB");

            HRESULT hr = ILDeserialize(bstr, &pidl);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);
            USHORT cb = *(USHORT*)((BYTE*)pidl);
            Assert::AreEqual((USHORT)4, cb);
            BYTE* abID = ((BYTE*)pidl) + sizeof(USHORT);
            Assert::AreEqual((BYTE)'A', abID[0]);
            Assert::AreEqual((BYTE)'B', abID[1]);
            USHORT term = *(USHORT*)((BYTE*)pidl + cb);
            Assert::AreEqual((USHORT)0, term);
            ::CoTaskMemFree(pidl);
            SysFreeString(bstr);
        }

        TEST_METHOD(ILDeserialize_MultipleItems)
        {
            LPITEMIDLIST pidl = nullptr;
            BSTR bstr = ::SysAllocString(L"X/YZ");

            HRESULT hr = ILDeserialize(bstr, &pidl);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);
            BYTE* cur = (BYTE*)pidl;
            USHORT cb1 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)3, cb1);
            Assert::AreEqual((BYTE)'X', cur[2]);
            cur += cb1;
            USHORT cb2 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)4, cb2);
            Assert::AreEqual((BYTE)'Y', cur[2]);
            Assert::AreEqual((BYTE)'Z', cur[3]);
            cur += cb2;
            USHORT term = *(USHORT*)cur;
            Assert::AreEqual((USHORT)0, term);
            ::CoTaskMemFree(pidl);
            SysFreeString(bstr);
        }

        TEST_METHOD(ILDeserialize_NullPointer_ReturnsPointerError)
        {
            BSTR bstr = ::SysAllocString(L"AB");
            HRESULT hr = ILDeserialize(bstr, nullptr);

            Assert::AreEqual(E_POINTER, hr);
            SysFreeString(bstr);
        }

        TEST_METHOD(ILSerializeThenDeserialize_RoundTrip_SingleItem)
        {
            BYTE abid1[] = { 'A', 'B' };
            const BYTE* abids[] = { abid1 };
            size_t abidLens[] = { sizeof(abid1) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 1);
            BSTR serialized = nullptr;
            LPITEMIDLIST roundTrip = nullptr;

            HRESULT hr1 = ILSerialize(pidl, serialized);
            HRESULT hr2 = ILDeserialize(serialized, &roundTrip);

            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);
            USHORT cb = *(USHORT*)((BYTE*)roundTrip);
            Assert::AreEqual((USHORT)4, cb);
            BYTE* abID = ((BYTE*)roundTrip) + sizeof(USHORT);
            Assert::AreEqual((BYTE)'A', abID[0]);
            Assert::AreEqual((BYTE)'B', abID[1]);
            ::SysFreeString(serialized);
            ::CoTaskMemFree(pidl);
            ::CoTaskMemFree(roundTrip);
        }

        TEST_METHOD(ILSerializeThenDeserialize_RoundTrip_MultiItem)
        {
            BYTE abid1[] = { 'X' };
            BYTE abid2[] = { 'Y', 'Z' };
            BYTE abid3[] = { '1', '2', '3' };
            const BYTE* abids[] = { abid1, abid2, abid3 };
            size_t abidLens[] = { sizeof(abid1), sizeof(abid2), sizeof(abid3) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 3);
            BSTR serialized = nullptr;
            LPITEMIDLIST roundTrip = nullptr;

            HRESULT hr1 = ILSerialize(pidl, serialized);
            HRESULT hr2 = ILDeserialize(serialized, &roundTrip);

            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);

            BYTE* cur = (BYTE*)roundTrip;
            USHORT cb1 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)3, cb1);
            Assert::AreEqual((BYTE)'X', cur[2]);
            cur += cb1;
            USHORT cb2 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)4, cb2);
            Assert::AreEqual((BYTE)'Y', cur[2]);
            Assert::AreEqual((BYTE)'Z', cur[3]);
            cur += cb2;
            USHORT cb3 = *(USHORT*)cur;
            Assert::AreEqual((USHORT)5, cb3);
            Assert::AreEqual((BYTE)'1', cur[2]);
            Assert::AreEqual((BYTE)'2', cur[3]);
            Assert::AreEqual((BYTE)'3', cur[4]);
            cur += cb3;
            USHORT term = *(USHORT*)cur;
            Assert::AreEqual((USHORT)0, term);

            ::SysFreeString(serialized);
            ::CoTaskMemFree(pidl);
            ::CoTaskMemFree(roundTrip);
        }

        TEST_METHOD(ILSerializeThenDeserialize_RoundTrip_8Items)
        {
            BYTE abid1[] = { 'A' };
            BYTE abid2[] = { 'B', 'C' };
            BYTE abid3[] = { 'D', 'E', 'F' };
            BYTE abid4[] = { 'G', 'H', 'I', 'J' };
            BYTE abid5[] = { 'K', 'L', 'M', 'N', 'O' };
            BYTE abid6[] = { 'P', 'Q', 'R', 'S', 'T', 'U' };
            BYTE abid7[] = { 'V', 'W', 'X', 'Y', 'Z', 'a', 'b' };
            BYTE abid8[] = { 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
            const BYTE* abids[] = { abid1, abid2, abid3, abid4, abid5, abid6, abid7, abid8 };
            size_t abidLens[] = { sizeof(abid1), sizeof(abid2), sizeof(abid3), sizeof(abid4), sizeof(abid5), sizeof(abid6), sizeof(abid7), sizeof(abid8) };
            LPITEMIDLIST pidl = CreateItemIdList(abids, abidLens, 8);
            BSTR serialized = nullptr;
            LPITEMIDLIST roundTrip = nullptr;

            HRESULT hr1 = ILSerialize(pidl, serialized);
            HRESULT hr2 = ILDeserialize(serialized, &roundTrip);

            Assert::AreEqual(S_OK, hr1);
            Assert::AreEqual(S_OK, hr2);

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

        TEST_METHOD(ILDeserialize_MoreThan16Items_HeapAllocFallback)
        {
            // 20 items, each abID is 'Q', separated by '/'
            std::wstring input;
            for (int i = 0; i < 20; ++i)
            {
                if (i > 0) input += L'/';
                input += L'Q';
            }

            BSTR bstr = ::SysAllocString(input.c_str());
            LPITEMIDLIST pidl = nullptr;

            HRESULT hr = ILDeserialize(bstr, &pidl);

            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(pidl != nullptr);

            BYTE* cur = (BYTE*)pidl;
            for (int i = 0; i < 20; ++i)
            {
                USHORT cb = *(USHORT*)cur;
                Assert::AreEqual((USHORT)3, cb);
                Assert::AreEqual((BYTE)'Q', cur[2]);
                cur += cb;
            }
            USHORT term = *(USHORT*)cur;
            Assert::AreEqual((USHORT)0, term);

            ::CoTaskMemFree(pidl);
            SysFreeString(bstr);
        }
    };
}