// <copyright file="ItemIdListTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <shtypes.h>

// Test
#include "CppUnitTest.h"

// BigDrive.ShellFolder
#include "ItemIdList.h"
#include "ShellItemId.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(ItemIdListTests)
    {
    private:
        /// <summary>
        /// Helper: Allocates a SHITEMID with the given abID data. Caller must free returned pointer with delete[].
        /// </summary>
        static SHITEMID* CreateSHITEMID(const BYTE* abID, USHORT abIDLen)
        {
            USHORT totalSize = static_cast<USHORT>(sizeof(USHORT) + abIDLen);
            BYTE* buffer = new BYTE[totalSize];
            SHITEMID* shitemid = reinterpret_cast<SHITEMID*>(buffer);
            shitemid->cb = totalSize;
            for (USHORT i = 0; i < abIDLen; ++i)
                shitemid->abID[i] = abID[i];
            return shitemid;
        }

        /// <summary>
        /// Helper: Allocates an ITEMIDLIST with the given SHITEMID array. Caller must free returned pointer with delete[].
        /// </summary>
        static LPITEMIDLIST CreateITEMIDLIST(const BYTE** abIDs, const USHORT* abIDLens, size_t count)
        {
            // Compute total size: sum of all SHITEMIDs + terminating zero SHITEMID
            size_t totalSize = 0;
            for (size_t i = 0; i < count; ++i)
                totalSize += sizeof(USHORT) + abIDLens[i];
            totalSize += sizeof(USHORT); // terminating SHITEMID

            BYTE* buffer = new BYTE[totalSize];
            BYTE* ptr = buffer;
            for (size_t i = 0; i < count; ++i)
            {
                USHORT cb = static_cast<USHORT>(sizeof(USHORT) + abIDLens[i]);
                *reinterpret_cast<USHORT*>(ptr) = cb;
                ptr += sizeof(USHORT);
                memcpy(ptr, abIDs[i], abIDLens[i]);
                ptr += abIDLens[i];
            }
            // Terminating SHITEMID
            *reinterpret_cast<USHORT*>(ptr) = 0;

            return reinterpret_cast<LPITEMIDLIST>(buffer);
        }

    public:
        /// <summary>
        /// Verifies that the hash of an empty ItemIdList is the seed value.
        /// </summary>
        TEST_METHOD(Hash_EmptyList_ReturnsSeed)
        {
            // Create an ITEMIDLIST with only the terminating SHITEMID
            BYTE buffer[sizeof(USHORT)] = {};
            LPITEMIDLIST pidl = reinterpret_cast<LPITEMIDLIST>(buffer);
            pidl->mkid.cb = 0;
            ItemIdList list(pidl);

            ULONG seed = 2166136261u;
            Assert::AreEqual(seed, list.Hash(seed));
        }

        /// <summary>
        /// Verifies that the hash of a single-item ItemIdList matches the hash of the contained ShellItemId.
        /// </summary>
        TEST_METHOD(Hash_SingleItem_EqualsShellItemIdHash)
        {
            BYTE abID[] = { 0x42, 0x43 };
            const BYTE* abIDs[] = { abID };
            USHORT abIDLens[] = { 2 };
            LPITEMIDLIST pidl = CreateITEMIDLIST(abIDs, abIDLens, 1);

            ItemIdList list(pidl);

            ULONG expected = 1007278677U;

            Assert::AreEqual(expected, list.Hash());

            delete[] reinterpret_cast<BYTE*>(pidl);
        }

        /// <summary>
        /// Verifies that the hash of a multi-item ItemIdList is not equal to the hash of any single item.
        /// </summary>
        TEST_METHOD(Hash_MultiItem_NotEqualToAnySingleItem)
        {
            BYTE abID0[] = { 0x01, 0x02 };
            BYTE abID1[] = { 0x03, 0x04 };
            BYTE abID2[] = { 0x05, 0x06 };
            const BYTE* abIDs[] = { abID0, abID1, abID2 };
            USHORT abIDLens[] = { 2, 2, 2 };

            LPITEMIDLIST pidl = CreateITEMIDLIST(abIDs, abIDLens, 3);

            ItemIdList list(pidl);
            ULONG hash = list.Hash();

            // Compute each ShellItemId hash
            const BYTE* ptr = reinterpret_cast<const BYTE*>(&pidl->mkid);
            for (int i = 0; i < 3; ++i)
            {
                const SHITEMID* shitemid = reinterpret_cast<const SHITEMID*>(ptr);
                ShellItemId shellItemId(shitemid);
                Assert::AreNotEqual(shellItemId.Hash(), hash);
                ptr += shitemid->cb;
            }

            delete[] reinterpret_cast<BYTE*>(pidl);
        }

        /// <summary>
        /// Verifies that different ItemIdLists produce different hashes.
        /// </summary>
        TEST_METHOD(Hash_DifferentLists_ProduceDifferentHashes)
        {
            BYTE abID1_0[] = { 0xAA, 0xBB };
            BYTE abID1_1[] = { 0xCC, 0xDD };
            BYTE abID2_0[] = { 0xAA, 0xBB };
            BYTE abID2_1[] = { 0xCC, 0xDE };
            const BYTE* abIDs1[] = { abID1_0, abID1_1 };
            const BYTE* abIDs2[] = { abID2_0, abID2_1 };
            USHORT abIDLens1[] = { 2, 2 };
            USHORT abIDLens2[] = { 2, 2 };

            LPITEMIDLIST pidl1 = CreateITEMIDLIST(abIDs1, abIDLens1, 2);
            LPITEMIDLIST pidl2 = CreateITEMIDLIST(abIDs2, abIDLens2, 2);

            ItemIdList list1(pidl1);
            ItemIdList list2(pidl2);

            Assert::AreNotEqual(list1.Hash(), list2.Hash());

            delete[] reinterpret_cast<BYTE*>(pidl1);
            delete[] reinterpret_cast<BYTE*>(pidl2);
        }

        /// <summary>
        /// Verifies that the hash is sensitive to the order of items in the ItemIdList.
        /// </summary>
        TEST_METHOD(Hash_OrderMatters)
        {
            BYTE abID1_0[] = { 0x01, 0x02 };
            BYTE abID1_1[] = { 0x03, 0x04 };
            BYTE abID2_0[] = { 0x03, 0x04 };
            BYTE abID2_1[] = { 0x01, 0x02 };
            const BYTE* abIDs1[] = { abID1_0, abID1_1 };
            const BYTE* abIDs2[] = { abID2_0, abID2_1 };
            USHORT abIDLens1[] = { 2, 2 };
            USHORT abIDLens2[] = { 2, 2 };

            LPITEMIDLIST pidl1 = CreateITEMIDLIST(abIDs1, abIDLens1, 2);
            LPITEMIDLIST pidl2 = CreateITEMIDLIST(abIDs2, abIDLens2, 2);

            ItemIdList list1(pidl1);
            ItemIdList list2(pidl2);

            Assert::AreNotEqual(list1.Hash(), list2.Hash());

            delete[] reinterpret_cast<BYTE*>(pidl1);
            delete[] reinterpret_cast<BYTE*>(pidl2);
        }
    };
}