// <copyright file="DllMainTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "CppUnitTest.h"

#include <windows.h>
#include <objbase.h>
#include <Unknwn.h> 

#include "..\..\..\src\BigDrive.ShellFolder\dllmain.h"

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
