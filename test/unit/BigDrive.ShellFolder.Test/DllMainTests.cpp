// <copyright file="DllMainTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "CppUnitTest.h"

#include <windows.h>
#include <objbase.h>
#include <Unknwn.h> 

#include "dllmain.h"

#include "RegistrationManagerExports.h" // Add this include

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveShellFolderTest
{
    TEST_CLASS(DllMainTests)
    {

    public:

        TEST_CLASS_INITIALIZE(Initialize)
        {
            Logger::WriteMessage(L"Initialize method called");

#ifdef _WIN64
            OutputDebugString(L"Test running as 64-bit\n");
#else
            OutputDebugString(L"Test running as 32-bit\n");
#endif
            // Load DLL explicitly
            HMODULE hModule = LoadLibrary(L"BigDrive.ShellFolder.dll");
            Assert::IsTrue(hModule != NULL, L"Failed to load DLL");

            HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            Assert::IsTrue(SUCCEEDED(hr), L"COM initialization failed");
        }

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

        /// <summary>
        /// Test DllGetClassObject with a valid CLSID.
        /// </summary>
        TEST_METHOD(TestValidCLSID)
        {
            // Arrange
            CLSID validCLSID = { 0xD4E5F6A7, 0xB8C9, 0x0123, { 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x12, 0x34 } };
            IID requestedIID = IID_IClassFactory;
            IClassFactory* pClassFactory = nullptr;

            RegisterShellFolderExport(validCLSID, L"TestShellFolder");

            // Act
            HRESULT hr = DllGetClassObject(validCLSID, requestedIID, reinterpret_cast<void**>(&pClassFactory));

            // Assert
            Assert::AreEqual(S_OK, hr, L"DllGetClassObject should return S_OK for a valid CLSID.");
            Assert::IsNotNull(pClassFactory, L"pClassFactory should not be null for a valid CLSID.");

            // Cleanup
            if (pClassFactory)
            {
                pClassFactory->Release();
            }
        }

        /// <summary>
        /// Test DllGetClassObject with an invalid CLSID.
        /// </summary>
        TEST_METHOD(TestInvalidCLSID)
        {
            // Arrange
            CLSID invalidCLSID = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            IID requestedIID = IID_IClassFactory;
            IClassFactory* pClassFactory = nullptr;

            // Act
            HRESULT hr = DllGetClassObject(invalidCLSID, requestedIID, reinterpret_cast<void**>(&pClassFactory));

            // Assert
            Assert::AreEqual(CLASS_E_CLASSNOTAVAILABLE, hr, L"DllGetClassObject should return CLASS_E_CLASSNOTAVAILABLE for an invalid CLSID.");
            Assert::IsNull(pClassFactory, L"pClassFactory should be null for an invalid CLSID.");
        }


        /// <summary>
        /// Test DllGetClassObject with a null ppv parameter.
        /// </summary>
        TEST_METHOD(TestNullPPV)
        {
            // Arrange
            CLSID validCLSID = { 0xD4E5F6A7, 0xB8C9, 0x0123, { 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x12, 0x34 } };
            IID requestedIID = IID_IClassFactory;

            // Act
            HRESULT hr = DllGetClassObject(validCLSID, requestedIID, nullptr);

            // Assert
            Assert::AreEqual(E_POINTER, hr, L"DllGetClassObject should return E_POINTER when ppv is null.");
        }

        /// <summary>
        /// Test DllGetClassObject with a valid CLSID but an unsupported IID.
        /// </summary>
        TEST_METHOD(TestUnsupportedIID)
        {
            // Arrange
            CLSID validCLSID = { 0xD4E5F6A7, 0xB8C9, 0x0123, { 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x12, 0x34 } };
            IID unsupportedIID = IID_IUnknown; // Assuming IID_IUnknown is not supported
            void* pUnknown = nullptr;

            BSTR bsrName = SysAllocString(L"TestShellFolder");

            // Mock RegistrationManager to return the valid CLSID
            RegisterShellFolderExport(validCLSID, bsrName);

            // Act
            HRESULT hr = DllGetClassObject(validCLSID, unsupportedIID, &pUnknown);

            // Assert
            Assert::AreEqual(E_NOINTERFACE, hr, L"DllGetClassObject should return E_NOINTERFACE for an unsupported IID.");
            Assert::IsNull(pUnknown, L"pUnknown should be null for an unsupported IID.");

            ::SysFreeString(bsrName);
        }

        // Add cleanup at the end of tests that use RegisterShellFolderExport
        TEST_METHOD_CLEANUP(CleanUp)
        {
            CleanUpShellFoldersExport(); // Clean up any test shell folders
            ::CoUninitialize();
        }
    };
}
