// <copyright file="SimpleTest.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "CppUnitTest.h"

//#include "..\..\..\src\IShellFolder\dllmain.h"

//#include "..\..\..\src\IShellFolder\RegistrationManagerExports.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(SimpleTest)
    {
        TEST_METHOD(Test)
        {
            Logger::WriteMessage(L"Test method called");
        }
    };
}