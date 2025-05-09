// <copyright file="SampleProviderTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"
#include "BigDriveInterfaceProvider.h"
#include "Interfaces/IBigDriveConfiguration.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveClientTest
{
    // CLSID declaration using const GUID
    const CLSID CLSID_BigDriveSampleProvider =
    { 0xF8FE2E5A, 0xE8B8, 0x4207, { 0xBC, 0x04, 0xEA, 0x4B, 0xCD, 0x4C, 0x43, 0x61 } };

    /// <summary>
    /// Use the Sample Provider To Test The Client
    /// </summary>
    TEST_CLASS(SampleProviderTests)
    {
    public:

        /// <summary>
        /// Test for GetIBigDriveRoot to ensure it retrieves the IBigDriveRoot interface successfully
        /// and calls GetRootFolders to verify the returned folder names.
        /// </summary>
        TEST_METHOD(TestGetIBigDriveRoot_Success)
        {
            // Arrange
            BigDriveInterfaceProvider provider(CLSID_BigDriveSampleProvider);
            IBigDriveRoot* pBigDriveRoot = nullptr;

            // Act
            HRESULT hr = provider.GetIBigDriveRoot(&pBigDriveRoot);

            // Assert
            Assert::AreEqual(S_OK, hr, L"GetIBigDriveRoot should return S_OK.");
            Assert::IsNotNull(pBigDriveRoot, L"pBigDriveRoot should not be null.");

            if (pBigDriveRoot)
            {
                // Call GetRootFolders
                GUID driveGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf2 } };
                SAFEARRAY* folders = nullptr;

                hr = pBigDriveRoot->GetRootFolders(driveGuid, &folders);

                // Assert the result of GetRootFolders
                Assert::AreEqual(S_OK, hr, L"GetRootFolders should return S_OK.");
                Assert::IsNotNull(folders, L"GetRootFolders should return a non-null SAFEARRAY.");

                // Access the SAFEARRAY elements
                if (folders)
                {
                    LONG lowerBound = 0, upperBound = 0;
                    SafeArrayGetLBound(folders, 1, &lowerBound);
                    SafeArrayGetUBound(folders, 1, &upperBound);

                    Assert::IsTrue(upperBound >= lowerBound, L"SAFEARRAY should have at least one element.");

                    for (LONG i = lowerBound; i <= upperBound; ++i)
                    {
                        BSTR folderName = nullptr;
                        SafeArrayGetElement(folders, &i, &folderName);

                        // Example: Check the first folder name (adjust as needed for your test data)
                        if (i == lowerBound)
                        {
                            Assert::AreEqual(L"RootFolder1", folderName, L"The first folder name should match.");
                        }

                        // Free the BSTR
                        SysFreeString(folderName);
                    }

                    // Destroy the SAFEARRAY
                    SafeArrayDestroy(folders);
                }
            }

            // Cleanup
            if (pBigDriveRoot)
            {
                pBigDriveRoot->Release();
                pBigDriveRoot = nullptr;
            }
        }
    };
}