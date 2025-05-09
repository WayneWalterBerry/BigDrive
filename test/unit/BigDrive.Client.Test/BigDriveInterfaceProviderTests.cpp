// <copyright file="BigDriveInterfaceProviderTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"
#include "BigDriveInterfaceProvider.h"
#include "Interfaces/IBigDriveConfiguration.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveClientTest
{
    TEST_CLASS(BigDriveInterfaceProviderTests)
    {
    public:
        /// <summary>
        /// Tests that BigDriveInterfaceProvider correctly identifies IID_IBigDriveConfiguration
        /// for the CLSID CLSID_BigDriveConfiguration.
        /// </summary>
        TEST_METHOD(VerifySupportedInterface)
        {
            // Arrange
            BigDriveInterfaceProvider provider(CLSID_BigDriveConfiguration);

            // Act
            IUnknown* pUnknown = nullptr;
            HRESULT hr = provider.GetInterface(IID_IBigDriveConfiguration, &pUnknown);

            // Assert
            Assert::AreEqual(S_OK, hr, L"Failed to retrieve IID_IBigDriveConfiguration");
        }


        /// <summary>
        /// Test for GetIBigDriveConfiguration to ensure it retrieves the IBigDriveConfiguration interface successfully.
        /// </summary>
        TEST_METHOD(TestGetIBigDriveConfiguration_Success)
        {
            // Arrange
            BigDriveInterfaceProvider provider(CLSID_BigDriveConfiguration);
            IBigDriveConfiguration* pBigDriveConfig = nullptr;

            // Act
            HRESULT hr = provider.GetIBigDriveConfiguration(&pBigDriveConfig);

            // Assert
            Assert::AreEqual(S_OK, hr, L"GetIBigDriveConfiguration should return S_OK.");
            Assert::IsNotNull(pBigDriveConfig, L"pBigDriveConfig should not be null.");

            // Cleanup
            if (pBigDriveConfig)
            {
                pBigDriveConfig->Release();
                pBigDriveConfig = nullptr;
            }
        }

        /// <summary>
        /// Test for GetIBigDriveConfiguration to ensure it handles null pointer input gracefully.
        /// </summary>
        TEST_METHOD(TestGetIBigDriveConfiguration_NullPointer)
        {
            // Arrange
            BigDriveInterfaceProvider provider(CLSID_BigDriveConfiguration);

            // Act
            HRESULT hr = provider.GetIBigDriveConfiguration(nullptr);

            // Assert
            Assert::AreEqual(E_POINTER, hr, L"GetIBigDriveConfiguration should return E_POINTER when passed a null pointer.");
        }

        /// <summary>
        /// Test for GetIBigDriveConfiguration to ensure it handles failure to retrieve the interface.
        /// </summary>
        TEST_METHOD(TestGetIBigDriveConfiguration_InterfaceNotFound)
        {
            // Arrange
            CLSID clsid = { /* Replace with an invalid CLSID or one that does not support IBigDriveConfiguration */ };
            BigDriveInterfaceProvider provider(clsid);
            IBigDriveConfiguration* pBigDriveConfig = nullptr;

            // Act
            HRESULT hr = provider.GetIBigDriveConfiguration(&pBigDriveConfig);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"GetIBigDriveConfiguration should not return S_OK for an unsupported CLSID.");
            Assert::IsNull(pBigDriveConfig, L"pBigDriveConfig should be null when the interface is not found.");
        }
    };
}
