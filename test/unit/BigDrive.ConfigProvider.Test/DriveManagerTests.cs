// <copyright file="ConfigurationProviderTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

using System;
using System.Threading;
using BigDrive.ConfigProvider.Model;
using Microsoft.Win32;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace BigDrive.Unit.ConfigurationProvider.Test
{
    /// <summary>
    /// Contains unit tests for the ConfigurationProvider class, which interacts with the Windows Registry
    /// to manage drive configurations.
    /// </summary>
    [TestClass]
    public class DriveManagerTests
    {
        private const string TestRegistryPath = @"Software\BigDrive\Drives";

        /// <summary>
        /// Initializes the test environment by setting up test registry keys.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
            using (RegistryKey testKey = Registry.CurrentUser.CreateSubKey(TestRegistryPath))
            {
                if (testKey != null)
                {
                    Guid driveGuid = Guid.NewGuid();
                    Guid clsid = Guid.NewGuid();
                    using (RegistryKey subKey = testKey.CreateSubKey(driveGuid.ToString("B")))
                    {
                        subKey.SetValue("id", driveGuid.ToString());
                        subKey.SetValue("name", "TestDrive");
                        subKey.SetValue("clsid", clsid.ToString());
                    }
                }
            }
        }

        /// <summary>
        /// Cleans up the test environment by removing test registry keys.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
            Registry.CurrentUser.DeleteSubKeyTree(TestRegistryPath, false);
        }

        /// <summary>
        /// Tests that ReadConfigurations returns a valid collection of configurations when the registry contains valid data.
        /// </summary>
        [TestMethod]
        public void ReadConfigurations_ValidInput_ReturnsConfigurations()
        {
            CancellationToken cancellationToken = CancellationToken.None;
            var configurations = BigDrive.ConfigProvider.DriveManager.ReadConfigurations(cancellationToken);
            Assert.IsNotNull(configurations);
            Assert.IsTrue(configurations.GetEnumerator().MoveNext());
        }

        /// <summary>
        /// Tests that ReadConfigurations throws an InvalidOperationException when the registry key is missing.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        public void ReadConfigurations_MissingRegistryKey_ThrowsException()
        {
            Registry.CurrentUser.DeleteSubKeyTree(TestRegistryPath, false);
            CancellationToken cancellationToken = CancellationToken.None;
            var configurations = BigDrive.ConfigProvider.DriveManager.ReadConfigurations(cancellationToken);
            Assert.IsNotNull(configurations);
            Assert.IsFalse(configurations.GetEnumerator().MoveNext());
        }

        /// <summary>
        /// Tests that ReadConfiguration returns a valid configuration when provided with a valid GUID.
        /// </summary>
        [TestMethod]
        public void ReadConfiguration_ValidInput_ReturnsConfiguration()
        {
            Guid driveGuid = Guid.NewGuid();
            Guid clsid = Guid.NewGuid();

            string subFolderRegistryPath = $@"{TestRegistryPath}\{driveGuid:B}";
            using (RegistryKey subKey = Registry.CurrentUser.CreateSubKey(subFolderRegistryPath))
            {
                subKey.SetValue("id", driveGuid.ToString());
                subKey.SetValue("name", "TestDrive");
                subKey.SetValue("clsid", clsid);
            }
            CancellationToken cancellationToken = CancellationToken.None;
            var configuration = BigDrive.ConfigProvider.DriveManager.ReadConfiguration(driveGuid, cancellationToken);
            Assert.IsNotNull(configuration);
            Assert.AreEqual(driveGuid, configuration.Id);
            Assert.AreEqual("TestDrive", configuration.Name);
            Assert.AreEqual(clsid, configuration.CLSID);
        }

        /// <summary>
        /// Tests that ReadConfiguration throws an InvalidOperationException when the registry key is missing.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        public void ReadConfiguration_MissingRegistryKey_ThrowsException()
        {
            Guid testGuid = Guid.NewGuid();
            CancellationToken cancellationToken = CancellationToken.None;
            BigDrive.ConfigProvider.DriveManager.ReadConfiguration(testGuid, cancellationToken);
        }

        /// <summary>
        /// Tests that ReadConfiguration throws an InvalidOperationException when the ID in the registry does not match the provided GUID.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        public void ReadConfiguration_IdMismatch_ThrowsException()
        {
            Guid testGuid = Guid.NewGuid();
            string subFolderRegistryPath = $@"{TestRegistryPath}\{testGuid:B}";
            using (RegistryKey subKey = Registry.CurrentUser.CreateSubKey(subFolderRegistryPath))
            {
                subKey.SetValue("id", Guid.NewGuid().ToString());
                subKey.SetValue("name", "TestDrive");
                subKey.SetValue("clsid", Guid.NewGuid().ToString());
            }
            CancellationToken cancellationToken = CancellationToken.None;
            BigDrive.ConfigProvider.DriveManager.ReadConfiguration(testGuid, cancellationToken);
        }

        /// <summary>
        /// Tests that ReadConfiguration throws an ArgumentNullException when provided with an empty GUID.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void ReadConfiguration_NullGuid_ThrowsException()
        {
            CancellationToken cancellationToken = CancellationToken.None;
            BigDrive.ConfigProvider.DriveManager.ReadConfiguration(Guid.Empty, cancellationToken);
        }

        /// <summary>
        /// Tests that WriteConfiguration writes a valid configuration to the registry.
        /// </summary>
        [TestMethod]
        public void WriteConfiguration_ValidInput_WritesToRegistry()
        {
            var driveConfig = new DriveConfiguration
            {
                Id = Guid.NewGuid(),
                Name = "TestDrive",
                CLSID = Guid.NewGuid()
            };

            CancellationToken cancellationToken = CancellationToken.None;
            BigDrive.ConfigProvider.DriveManager.WriteConfiguration(driveConfig, cancellationToken);
            string subFolderRegistryPath = $@"{TestRegistryPath}\{driveConfig.Id:B}";
            using (RegistryKey subKey = Registry.CurrentUser.OpenSubKey(subFolderRegistryPath))
            {
                Assert.IsNotNull(subKey);
                Assert.AreEqual(driveConfig.Id.ToString(), subKey.GetValue("id"));
                Assert.AreEqual(driveConfig.Name, subKey.GetValue("name"));
                Assert.AreEqual(driveConfig.CLSID.ToString(), subKey.GetValue("clsid"));
            }
        }

        /// <summary>
        /// Tests that WriteConfiguration throws an ArgumentNullException when provided with a null DriveConfiguration.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void WriteConfiguration_NullDriveConfig_ThrowsException()
        {
            CancellationToken cancellationToken = CancellationToken.None;
            BigDrive.ConfigProvider.DriveManager.WriteConfiguration(null, cancellationToken);
        }
    }
}

