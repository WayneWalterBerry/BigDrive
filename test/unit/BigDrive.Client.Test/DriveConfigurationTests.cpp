// <copyright file="DriveConfigurationTests.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "CppUnitTest.h"
#include "..\..\src\BigDrive.Client\DriveConfiguration.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace BigDriveClientTest
{
    TEST_CLASS(DriveConfigurationTests)
    {
    public:

        DriveConfigurationTests()
        {
            ::EnableMemoryLeakChecks();
        }

        /// <summary>
        /// Test ParseJson with valid JSON input.
        /// </summary>
        TEST_METHOD(TestParseJson_ValidJson)
        {
            // Arrange
            DriveConfiguration config;
            LPCWSTR validJson = L"{\"id\":\"12345678-1234-1234-1234-56789abcdef2\",\"name\":\"TestDrive\",\"clsid\":\"12345678-1234-1234-1234-56789abc01f2\"}";

            // Act
            HRESULT hr = config.ParseJson(validJson);

            // Assert
            Assert::AreEqual(S_OK, hr, L"ParseJson should return S_OK for valid JSON.");

            Assert::IsTrue(
                GUID{ 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf2 } } == config.id,
                L"ID should match the parsed GUID.");

            Assert::AreEqual(wstring(L"TestDrive"), wstring(config.name), L"Name should match the parsed value.");

            Assert::IsTrue(
                GUID{ 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0x01, 0xf2 } } == config.clsid,
                L"CLSID should match the parsed GUID.");
        }

        /// <summary>
        /// Test ParseJson with invalid JSON input.
        /// </summary>
        TEST_METHOD(TestParseJson_InvalidJson)
        {
            // Arrange
            DriveConfiguration config;
            LPCWSTR invalidJson = L"{\"id\":\"invalid-guid\",\"name\":\"TestDrive\",\"clsid\":\"12345678-1234-1234-1234-56789abc01f2\"}";

            // Act
            HRESULT hr = config.ParseJson(invalidJson);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"ParseJson should not return S_OK for invalid JSON.");
        }

        /// <summary>
        /// Test ParseJson with missing fields in JSON input.
        /// </summary>
        TEST_METHOD(TestParseJson_MissingFields)
        {
            // Arrange
            DriveConfiguration config;
            LPCWSTR missingFieldsJson = L"{\"id\":\"12345678-1234-1234-1234-56789abcdef2\"}";

            // Act
            HRESULT hr = config.ParseJson(missingFieldsJson);

            // Assert
            Assert::AreEqual(S_OK, hr, L"ParseJson should return S_OK even if some fields are missing.");

            Assert::IsTrue(GUID{ 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf2 } } == config.id,
                L"ID should match the parsed GUID.");

            Assert::IsNull(config.name, L"Name should be null if not present in the JSON.");

            Assert::IsTrue(GUID_NULL == config.clsid,
                L"CLSID should be GUID_NULL if not present in the JSON.");
        }

        /// <summary>
        /// Test ParseJson with null input.
        /// </summary>
        TEST_METHOD(TestParseJson_NullInput)
        {
            // Arrange
            DriveConfiguration config;

            // Act
            HRESULT hr = config.ParseJson(nullptr);

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr, L"ParseJson should return E_INVALIDARG for null input.");
        }

        /// <summary>
        /// Test ParseJson with empty JSON input.
        /// </summary>
        TEST_METHOD(TestParseJson_EmptyJson)
        {
            // Arrange
            DriveConfiguration config;
            LPCWSTR emptyJson = L"";

            // Act
            HRESULT hr = config.ParseJson(emptyJson);

            // Assert
            Assert::AreNotEqual(S_OK, hr, L"ParseJson should not return S_OK for empty JSON.");
        }
    };
}
