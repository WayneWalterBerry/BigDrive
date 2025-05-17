// <copyright file="ShellItemIdTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <shtypes.h>

// Test
#include "CppUnitTest.h"

// BigDrive.ShellFolder
#include "ShellItemId.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(ShellItemIdTests)
    {
    private:
        /// <summary>
        /// Allocates a SHITEMID structure with the given abID data.
        /// The caller is responsible for freeing the returned pointer with delete[].
        /// </summary>
        /// <param name="abID">Pointer to abID data.</param>
        /// <param name="abIDLen">Length of abID data.</param>
        /// <returns>Pointer to a newly allocated SHITEMID structure.</returns>
        static SHITEMID* CreateSHITEMID(const BYTE* abID, USHORT abIDLen)
        {
            // SHITEMID is variable size: sizeof(USHORT) + abIDLen
            USHORT totalSize = static_cast<USHORT>(sizeof(USHORT) + abIDLen);
            BYTE* buffer = new BYTE[totalSize];
            SHITEMID* shitemid = reinterpret_cast<SHITEMID*>(buffer);
            shitemid->cb = totalSize;
            for (USHORT i = 0; i < abIDLen; ++i)
            {
                shitemid->abID[i] = abID[i];
            }
            return shitemid;
        }

    public:

        /// <summary>
        /// Verifies that the hash of an empty SHITEMID (cb = 0) is zero.
        /// </summary>
        TEST_METHOD(Hash_EmptySHITEMID_ReturnsZero)
        {
            // cb = 0 means no abID data
            SHITEMID shitemid = { 0 };
            ShellItemId item(&shitemid);
            Assert::AreEqual(static_cast<ULONG>(0), item.Hash());
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with a single abID byte returns the expected FNV-1a value.
        /// </summary>
        TEST_METHOD(Hash_OneByteSHITEMID_ReturnsExpected)
        {
            BYTE abID[] = { 0x42 };
            SHITEMID* shitemid = CreateSHITEMID(abID, 1);
            ShellItemId item(shitemid);

            // FNV-1a 32-bit: offset basis 2166136261, prime 16777619
            // hash = ((2166136261 ^ 0x42) * 16777619) = 36342607848930181
            ULONG expected = 36342607848930181u;

            Assert::AreEqual(expected, item.Hash());

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with multiple abID bytes returns the expected FNV-1a value.
        /// </summary>
        TEST_METHOD(Hash_MultiByteSHITEMID_ReturnsExpected)
        {
            BYTE abID[] = { 0x01, 0x02, 0x03 };
            SHITEMID* shitemid = CreateSHITEMID(abID, 3);
            ShellItemId item(shitemid);

            // Manual FNV-1a 32-bit calculation:
            // hash = 2166136261
            // hash ^= 0x01; hash *= 16777619;
            // hash ^= 0x02; hash *= 16777619;
            // hash ^= 0x03; hash *= 16777619;
            ULONG hash = 2166136261u;
            hash ^= 0x01; hash *= 16777619u;
            hash ^= 0x02; hash *= 16777619u;
            hash ^= 0x03; hash *= 16777619u;
            ULONG expected = hash;

            Assert::AreEqual(expected, item.Hash());

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that two SHITEMIDs with different abID content produce different hash values.
        /// </summary>
        TEST_METHOD(Hash_DifferentContent_ProducesDifferentHash)
        {
            BYTE abID1[] = { 0xAA, 0xBB };
            BYTE abID2[] = { 0xAA, 0xBC };
            SHITEMID* shitemid1 = CreateSHITEMID(abID1, 2);
            SHITEMID* shitemid2 = CreateSHITEMID(abID2, 2);

            ShellItemId item1(shitemid1);
            ShellItemId item2(shitemid2);

            size_t hash1 = item1.Hash();
            size_t hash2 = item2.Hash();

            Assert::AreNotEqual(hash1, hash2);

            delete[] reinterpret_cast<BYTE*>(shitemid1);
            delete[] reinterpret_cast<BYTE*>(shitemid2);
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with zero-length abID (cb = sizeof(USHORT)) is zero.
        /// </summary>
        TEST_METHOD(Hash_ZeroLength_abID_ReturnsZero)
        {
            // cb = sizeof(USHORT), abID is zero-length
            BYTE buffer[sizeof(USHORT)] = {};
            SHITEMID* shitemid = reinterpret_cast<SHITEMID*>(buffer);
            shitemid->cb = sizeof(USHORT);
            ShellItemId item(shitemid);
            Assert::AreEqual(static_cast<ULONG>(0), item.Hash());
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with all abID bytes set to zero is not zero.
        /// </summary>
        TEST_METHOD(Hash_AllZero_abID)
        {
            BYTE abID[] = { 0x00, 0x00, 0x00, 0x00 };
            SHITEMID* shitemid = CreateSHITEMID(abID, 4);
            ShellItemId item(shitemid);

            ULONG hash = item.Hash();

            // Should not be zero (unless FNV-1a produces zero for this input, which it does not)
            Assert::AreNotEqual(static_cast<ULONG>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with all abID bytes set to 0xFF is not zero.
        /// </summary>
        TEST_METHOD(Hash_AllFF_abID)
        {
            BYTE abID[] = { 0xFF, 0xFF, 0xFF, 0xFF };
            SHITEMID* shitemid = CreateSHITEMID(abID, 4);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash function works for a large abID (256 bytes).
        /// </summary>
        TEST_METHOD(Hash_Large_abID)
        {
            const USHORT abIDLen = 256;
            BYTE abID[abIDLen];
            for (USHORT i = 0; i < abIDLen; ++i)
                abID[i] = static_cast<BYTE>(i);
            SHITEMID* shitemid = CreateSHITEMID(abID, abIDLen);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that SHITEMIDs with a single abID byte of different values produce different hashes.
        /// </summary>
        TEST_METHOD(Hash_SingleByteDifferentValues)
        {
            for (BYTE v = 0; v < 10; ++v)
            {
                BYTE abID[] = { v };
                SHITEMID* shitemid = CreateSHITEMID(abID, 1);
                ShellItemId item(shitemid);
                size_t hash = item.Hash();
                // All hashes should be different for different v
                for (BYTE w = 0; w < v; ++w)
                {
                    BYTE abID2[] = { w };
                    SHITEMID* shitemid2 = CreateSHITEMID(abID2, 1);
                    ShellItemId item2(shitemid2);
                    size_t hash2 = item2.Hash();
                    Assert::AreNotEqual(hash, hash2);
                    delete[] reinterpret_cast<BYTE*>(shitemid2);
                }
                delete[] reinterpret_cast<BYTE*>(shitemid);
            }
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with a repeated abID pattern is not zero.
        /// </summary>
        TEST_METHOD(Hash_RepeatedPattern_abID)
        {
            BYTE abID[] = { 0xAB, 0xCD, 0xAB, 0xCD, 0xAB, 0xCD };
            SHITEMID* shitemid = CreateSHITEMID(abID, 6);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with incrementing abID values is not zero.
        /// </summary>
        TEST_METHOD(Hash_Incrementing_abID)
        {
            BYTE abID[16];
            for (BYTE i = 0; i < 16; ++i)
                abID[i] = i + 1;
            SHITEMID* shitemid = CreateSHITEMID(abID, 16);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with decrementing abID values is not zero.
        /// </summary>
        TEST_METHOD(Hash_Decrementing_abID)
        {
            BYTE abID[16];
            for (BYTE i = 0; i < 16; ++i)
                abID[i] = 16 - i;
            SHITEMID* shitemid = CreateSHITEMID(abID, 16);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash is sensitive to the position of a single set bit in abID.
        /// </summary>
        TEST_METHOD(Hash_OneBitSet_abID)
        {
            for (BYTE i = 0; i < 8; ++i)
            {
                BYTE abID[] = { static_cast<BYTE>(1 << i), 0x00, 0x00 };
                SHITEMID* shitemid = CreateSHITEMID(abID, 3);
                ShellItemId item(shitemid);
                size_t hash = item.Hash();

                for (BYTE j = 0; j < i; ++j)
                {
                    BYTE abID2[] = { static_cast<BYTE>(1 << j), 0x00, 0x00 };
                    SHITEMID* shitemid2 = CreateSHITEMID(abID2, 3);
                    ShellItemId item2(shitemid2);
                    size_t hash2 = item2.Hash();
                    Assert::AreNotEqual(hash, hash2);
                    delete[] reinterpret_cast<BYTE*>(shitemid2);
                }
                delete[] reinterpret_cast<BYTE*>(shitemid);
            }
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with non-ASCII abID values is not zero.
        /// </summary>
        TEST_METHOD(Hash_NonAscii_abID)
        {
            BYTE abID[] = { 0x80, 0xFE, 0xC3, 0xA9 };
            SHITEMID* shitemid = CreateSHITEMID(abID, 4);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }

        /// <summary>
        /// Verifies that the hash of a SHITEMID with null bytes in the middle of abID is not zero.
        /// </summary>
        TEST_METHOD(Hash_NullBytesInMiddle)
        {
            BYTE abID[] = { 0x12, 0x00, 0x34, 0x00, 0x56 };
            SHITEMID* shitemid = CreateSHITEMID(abID, 5);
            ShellItemId item(shitemid);

            size_t hash = item.Hash();
            Assert::AreNotEqual(static_cast<size_t>(0), hash);

            delete[] reinterpret_cast<BYTE*>(shitemid);
        }
    };
}