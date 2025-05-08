#include "pch.h"
#include "CppUnitTest.h"
#include "GuidUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BigDriveClient;

namespace BigDriveClientTest
{
    TEST_CLASS(GuidUtilTests)
    {
    public:

        /// <summary>
        /// Tests the StringFromGUID method to ensure it correctly converts a GUID to a string without braces.
        /// </summary>
        TEST_METHOD(TestStringFromGUID)
        {
            // Arrange
            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            wchar_t guidWithoutBraces[37]; // GUID without braces is 36 characters + null terminator

            // Act
            HRESULT hr = StringFromGUID(testGuid, guidWithoutBraces, ARRAYSIZE(guidWithoutBraces));

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::AreEqual(L"12345678-1234-1234-1234-56789ABCDEF0", guidWithoutBraces);
        }

        /// <summary>
        /// Tests the GUIDFromString method to ensure it correctly converts a string to a GUID.
        /// </summary>
        TEST_METHOD(TestGUIDFromString)
        {
            // Arrange
            const wchar_t* guidString = L"12345678-1234-1234-1234-56789abcdef0";
            GUID expectedGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            GUID actualGuid;

            // Act
            HRESULT hr = GUIDFromString(guidString, &actualGuid);

            // Assert
            Assert::AreEqual(S_OK, hr);
            Assert::IsTrue(IsEqualGUID(expectedGuid, actualGuid));
        }

        /// <summary>
        /// Tests the StringFromGUID method with a buffer that is too small to ensure it returns an error.
        /// </summary>
        TEST_METHOD(TestStringFromGUID_BufferTooSmall)
        {
            // Arrange
            GUID testGuid = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };
            wchar_t smallBuffer[10]; // Intentionally too small

            // Act
            HRESULT hr = StringFromGUID(testGuid, smallBuffer, ARRAYSIZE(smallBuffer));

            // Assert
            Assert::AreEqual(E_FAIL, hr);
        }

        /// <summary>
        /// Tests the GUIDFromString method with an invalid string to ensure it returns an error.
        /// </summary>
        TEST_METHOD(TestGUIDFromString_InvalidString)
        {
            // Arrange
            const wchar_t* invalidGuidString = L"Invalid-GUID-String";
            GUID actualGuid;

            // Act
            HRESULT hr = GUIDFromString(invalidGuidString, &actualGuid);

            // Assert
            Assert::AreEqual(E_FAIL, hr);
        }

        /// <summary>
        /// Tests the GUIDFromString method with a null input to ensure it returns an error.
        /// </summary>
        TEST_METHOD(TestGUIDFromString_NullInput)
        {
            // Arrange
            GUID actualGuid;

            // Act
            HRESULT hr = GUIDFromString(nullptr, &actualGuid);

            // Assert
            Assert::AreEqual(E_INVALIDARG, hr);
        }
    };
}
