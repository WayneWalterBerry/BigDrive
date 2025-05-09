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
            std::vector<IID> interfaceIDs;

            // Act
            HRESULT hr = provider.GetSupportedInterfaceIDs(interfaceIDs);

            // Assert
            Assert::AreEqual(S_OK, hr, L"Failed to retrieve supported interface IDs.");
            bool found = false;

            for (const auto& iid : interfaceIDs)
            {
                if (IsEqualIID(iid, IID_IBigDriveConfiguration))
                {
                    found = true;
                    break;
                }
            }

            Assert::IsTrue(found, L"IID_IBigDriveConfiguration was not found in the supported interfaces.");
        }
    };
}
