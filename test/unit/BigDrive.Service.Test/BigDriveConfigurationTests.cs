// <copyright file="BigDriveConfigurationTests.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
// Unit tests for the BigDriveConfiguration class, ensuring that configurations
// written using ConfigurationProvider.WriteConfiguration are correctly retrieved
// using the GetConfiguration method.
// </summary>

using System;
using System.Text.Json;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using BigDrive.Service.ComObjects;
using BigDrive.ConfigProvider;
using BigDrive.ConfigProvider.Model;
using Microsoft.Win32;

namespace BigDrive.Unit.Service.Test
{
    /// <summary>
    /// Contains unit tests for the <see cref="BigDriveConfiguration"/> class.
    /// </summary>
    [TestClass]
    public class BigDriveConfigurationTests
    {
        private const string TestRegistryPath = @"Software\BigDrive\Drives";

        /// <summary>
        /// Cleans up the test environment by removing test registry keys.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
            Registry.CurrentUser.DeleteSubKeyTree(TestRegistryPath, false);
        }

        /// <summary>
        /// Tests that GetConfiguration retrieves the correct configuration
        /// written using ConfigurationProvider.WriteConfiguration.
        /// </summary>
        [TestMethod]
        public void GetConfiguration_ValidInput_ReturnsCorrectConfiguration()
        {
            // Arrange
            var driveConfig = new DriveConfiguration
            {
                Id = Guid.NewGuid(),
                Name = "TestDrive"
            };
            CancellationToken cancellationToken = CancellationToken.None;

            // Write the configuration to the registry
            ConfigurationProvider.WriteConfiguration(driveConfig, cancellationToken);

            // Act
            var bigDriveConfiguration = new BigDriveConfiguration();
            string json = bigDriveConfiguration.GetConfiguration(driveConfig.Id);

            // Deserialize the JSON back to a DriveConfiguration object
            var options = new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true
            };
            var retrievedConfig = JsonSerializer.Deserialize<DriveConfiguration>(json, options);

            // Assert
            Assert.IsNotNull(retrievedConfig);
            Assert.AreEqual(driveConfig.Id, retrievedConfig.Id);
            Assert.AreEqual(driveConfig.Name, retrievedConfig.Name);
        }

        /// <summary>
        /// Tests that GetConfiguration throws an exception when the configuration
        /// for the given GUID does not exist.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        public void GetConfiguration_MissingConfiguration_ThrowsException()
        {
            // Arrange
            var bigDriveConfiguration = new BigDriveConfiguration();
            Guid missingGuid = Guid.NewGuid();

            // Act
            bigDriveConfiguration.GetConfiguration(missingGuid);
        }
    }
}
