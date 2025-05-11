using Microsoft.VisualStudio.TestTools.UnitTesting;
using BigDrive.ConfigProvider;
using BigDrive.ConfigProvider.Model;
using Microsoft.Win32;
using System;
using System.Threading;

namespace BigDrive.Unit.ConfigurationProvider.Test
{
    [TestClass]
    public class ProviderManagerTests
    {
        private const string TestRegistryBasePath = @"Software\BigDrive\Providers";

        [TestCleanup]
        public void Cleanup()
        {
            // Clean up the test registry path after each test
            using (RegistryKey baseKey = Registry.CurrentUser.OpenSubKey(TestRegistryBasePath, true))
            {
                if (baseKey != null)
                {
                    Registry.CurrentUser.DeleteSubKeyTree(TestRegistryBasePath, false);
                }
            }
        }

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
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void RegisterProvider_NullProviderConfiguration_ThrowsArgumentNullException()
        {
            // Act
            ProviderManager.RegisterProvider(null, CancellationToken.None);
        }

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
