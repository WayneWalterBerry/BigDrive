// <copyright file="ProviderManagerTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
// Unit tests for the ProviderManager class in the BigDrive.ConfigProvider namespace.
// These tests validate the functionality of registering and unregistering providers
// in the Windows registry under the HKEY_CURRENT_USER hive.
// </summary>

namespace BigDrive.Unit.ConfigurationProvider.Test
{
    using System;
    using System.Threading;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;
    using Microsoft.Win32;

    /// <summary>
    /// Unit tests for the ProviderManager class.
    /// These tests ensure that providers are correctly registered and unregistered
    /// in the Windows registry under the HKEY_CURRENT_USER hive.
    /// </summary>
    [TestClass]
    public class ProviderManagerTests
    {
        private const string TestRegistryBasePath = @"Software\BigDrive\Providers";

        /// <summary>
        /// Cleans up the test registry path after each test to ensure a clean state.
        /// </summary>
        [TestCleanup]
        public void Cleanup()
        {
            using (RegistryKey baseKey = Registry.CurrentUser.OpenSubKey(TestRegistryBasePath, true))
            {
                if (baseKey != null)
                {
                    Registry.CurrentUser.DeleteSubKeyTree(TestRegistryBasePath, false);
                }
            }
        }

        /// <summary>
        /// Tests that a valid ProviderConfiguration is correctly written to the registry.
        /// </summary>
        [TestMethod]
        public void RegisterProvider_ValidProviderConfiguration_WritesToRegistry()
        {
            // Arrange
            var providerConfig = new ProviderConfiguration
            {
                Id = Guid.NewGuid(),
                Name = "TestProvider"
            };

            // Act
            ProviderManager.RegisterProvider(providerConfig, CancellationToken.None);

            // Assert
            string registryPath = $@"{TestRegistryBasePath}\{{{providerConfig.Id}}}";
            using (RegistryKey key = Registry.CurrentUser.OpenSubKey(registryPath))
            {
                Assert.IsNotNull(key, "Registry key was not created.");
                Assert.AreEqual(providerConfig.Name, key.GetValue("name"), "Registry value was not set correctly.");
            }

            ProviderManager.UnRegisterProvider(providerConfig.Id, CancellationToken.None);
        }

        /// <summary>
        /// Tests that passing a null ProviderConfiguration to RegisterProvider throws an ArgumentNullException.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void RegisterProvider_NullProviderConfiguration_ThrowsArgumentNullException()
        {
            // Act
            ProviderManager.RegisterProvider(null, CancellationToken.None);
        }

        /// <summary>
        /// Tests that a valid provider GUID is correctly unregistered from the registry.
        /// </summary>
        [TestMethod]
        public void UnRegisterProvider_ValidGuid_DeletesRegistryKey()
        {
            // Arrange
            var providerId = Guid.NewGuid();
            string registryPath = $@"{TestRegistryBasePath}\{{{providerId}}}";

            // Create a test registry key
            using (RegistryKey key = Registry.CurrentUser.CreateSubKey(registryPath))
            {
                key.SetValue("name", "TestProvider");
            }

            // Act
            ProviderManager.UnRegisterProvider(providerId, CancellationToken.None);

            // Assert
            using (RegistryKey key = Registry.CurrentUser.OpenSubKey(registryPath))
            {
                Assert.IsNull(key, "Registry key was not deleted.");
            }
        }

        /// <summary>
        /// Tests that attempting to unregister a non-existent provider GUID does not throw an exception.
        /// </summary>
        [TestMethod]
        public void UnRegisterProvider_NonExistentGuid_DoesNotThrow()
        {
            // Arrange
            var providerId = Guid.NewGuid();

            // Act & Assert
            ProviderManager.UnRegisterProvider(providerId, CancellationToken.None);
        }
    }
}
