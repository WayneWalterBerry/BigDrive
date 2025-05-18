// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "CppUnitTest.h"

#include <windows.h>
#include <objbase.h>
#include <Unknwn.h> 

#include "dllmain.h"

#include "RegistrationManagerExports.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(SimpleTest)
    {
        TEST_CLASS_INITIALIZE(Initialize)
        {
            Logger::WriteMessage(L"Initialize method called");

#ifdef _WIN64
            Logger::WriteMessage(L"Test running as 64-bit\n");
#else
            Logger::WriteMessage(L"Test running as 32-bit\n");
#endif
            // Load DLL explicitly
            HMODULE hModule = LoadLibrary(L"BigDrive.ShellFolder.dll");
            Assert::IsTrue(hModule != NULL, L"Failed to load DLL");

            HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            Assert::IsTrue(SUCCEEDED(hr), L"COM initialization failed");
        }

        TEST_METHOD(Test)
        {
            Logger::WriteMessage(L"Test method called");
        }
    };
}