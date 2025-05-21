// <copyright file="ETWManifestManagerTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <windows.h>
#include "CppUnitTest.h"

#include "..\..\..\src\IShellFolder\Exports\ETWManifestManagerImports.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(ETWManifestManagerTests)
    {
    public:

        /// <summary>
        /// Tests that RegisterManifest successfully registers the ETW manifest.
        /// </summary>
        TEST_METHOD(RegisterManifest)
        {
            // Act
            HRESULT hr = ::RegisterManifest();

            // Assert
            Assert::AreEqual(S_OK, hr, L"RegisterManifest did not return S_OK.");

            // Optionally, you could add further verification here if you have a way to check
            // that the manifest is actually registered (e.g., by querying the registry or using wevtutil).
            // For now, we only check the HRESULT.
        }

        /// <summary>
        /// Tests that UnregisterManifest successfully registers the ETW manifest.
        /// </summary>
        TEST_METHOD(UnregisterManifest)
        {
            // Act
            HRESULT hr = ::UnregisterManifest();

            // Assert
            Assert::AreEqual(S_OK, hr, L"UnregisterManifest did not return S_OK.");

            // Optionally, you could add further verification here if you have a way to check
            // that the manifest is actually unregistered (e.g., by querying the registry or using wevtutil).
            // For now, we only check the HRESULT.
        }
    };
}