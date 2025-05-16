// <copyright file="DllMainTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "CppUnitTest.h"

#include "dllmain.h"

// Forward declare DllRegisterServer
extern "C" HRESULT __stdcall DllRegisterServer();

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(DllMainTests)
    {
    public:
        /// <summary>
        /// Test that DllRegisterServer returns S_OK.
        /// </summary>
        TEST_METHOD(DllRegisterServer_ReturnsS_OK)
        {
            // Act
            HRESULT hr = DllRegisterServer();

            // Assert
            Assert::AreEqual(S_OK, hr, L"DllRegisterServer did not return S_OK.");
        }
    };
}
