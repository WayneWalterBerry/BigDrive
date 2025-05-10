// <copyright file="DriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text.Json.Serialization;
    using System.Threading;
    using BigDrive.ConfigProvider.Model;
    using Microsoft.Win32;

    public static class ConfigurationProvider
    {
        /// <summary>
        /// Reads the Drives subfolder in the registry and iterates all subfolders, calling LoadConfiguration for each.
        /// </summary>
        public static IEnumerable<DriveConfiguration> ReadConfigurations(CancellationToken cancellationToken)
        {
            System.Diagnostics.Debugger.Launch();

            string drivesRegistryPath = Path.Combine("Software\\BigDrive", "Drives");

            using (RegistryKey drivesKey = Registry.CurrentUser.OpenSubKey(@"Software\BigDrive\Drives"))
            {
                if (drivesKey == null)
                {
                    throw new InvalidOperationException($"Registry path '{drivesRegistryPath}' does not exist.");
                }

                foreach (string subKeyName in drivesKey.GetSubKeyNames())
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    yield return ReadConfiguration(Guid.Parse(subKeyName), cancellationToken);
                }
            }
        }

        /// <summary>
        /// Loads a configuration for a specific subfolder name.
        /// </summary>
        /// <param name="subFolderName">The name of the subfolder in the registry.</param>
        /// <param name="cancellationToken">Cancellation Token</param>
        public static DriveConfiguration ReadConfiguration(Guid guid, CancellationToken cancellationToken)
        {
            System.Diagnostics.Debugger.Launch();

            if (guid== Guid.Empty)
            {
                throw new ArgumentNullException(nameof(guid), "Guid cannot be empty.");
            }

            // Define the registry path for the specific subfolder
            string subFolderRegistryPath = $@"Software\BigDrive\Drives\{guid:B}";

            using (RegistryKey subFolderKey = Registry.CurrentUser.OpenSubKey(subFolderRegistryPath))
            {
                if (subFolderKey == null)
                {
                    throw new InvalidOperationException($"Registry path '{subFolderRegistryPath}' does not exist.");
                }

                // Create an instance of DriveConfiguration
                var driveConfiguration = new DriveConfiguration();

                // Use reflection to get all properties of the DriveConfiguration class
                var properties = typeof(DriveConfiguration).GetProperties();

                foreach (var property in properties)
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    // Get the JSON property name (if it exists) or fall back to the property name
                    var jsonPropertyAttribute = property.GetCustomAttributes(typeof(JsonPropertyNameAttribute), false)
                                                        .FirstOrDefault() as JsonPropertyNameAttribute;
                    string registryValueName = jsonPropertyAttribute?.Name ?? property.Name;

                    // Get the value from the registry
                    object registryValue = subFolderKey.GetValue(registryValueName);

                    if (registryValue != null)
                    {
                        // Convert the registry value to the property type and set it
                        if (property.PropertyType == typeof(Guid))
                        {
                            property.SetValue(driveConfiguration, Guid.Parse(registryValue.ToString()));
                        }
                        else if (property.PropertyType == typeof(string))
                        {
                            property.SetValue(driveConfiguration, registryValue.ToString());
                        }
                        else if (property.PropertyType.IsEnum)
                        {
                            property.SetValue(driveConfiguration, Enum.Parse(property.PropertyType, registryValue.ToString()));
                        }
                        else
                        {
                            // For other types, use Convert.ChangeType This may need to be adjusted for specific types
                            // such as DateTime or custom types
                            property.SetValue(driveConfiguration, Convert.ChangeType(registryValue, property.PropertyType));
                        }
                    }
                }

                // Verify that the subfolder name matches the driveConfig.Id
                if (guid != driveConfiguration.Id)
                {
                    throw new InvalidOperationException($"Subfolder name of register key: '{subFolderKey.Name}' does not match Id found in that key '{driveConfiguration.Id}'.");
                }

                // Return the populated DriveConfiguration object
                return driveConfiguration;
            }
        }

        /// <summary>
        /// Writes a DriveConfiguration to the registry under the specified subfolder name.
        /// </summary>
        /// <param name="driveConfig">The DriveConfiguration object to write.</param>
        /// <param name="cancellationToken">Cancellation Token</param>
        public static void WriteConfiguration(DriveConfiguration driveConfig, CancellationToken cancellationToken)
        {
            if (driveConfig == null)
            {
                throw new ArgumentNullException(nameof(driveConfig), "DriveConfiguration cannot be null.");
            }

            // Define the registry path for the specific subfolder
            string subFolderRegistryPath = $@"Software\BigDrive\Drives\{{{driveConfig.Id}}}";

            using (RegistryKey subFolderKey = Registry.CurrentUser.CreateSubKey(subFolderRegistryPath))
            {
                if (subFolderKey == null)
                {
                    throw new InvalidOperationException($"Failed to create or open registry path '{subFolderRegistryPath}'.");
                }

                // Use reflection to get all properties of the DriveConfiguration class
                var properties = typeof(DriveConfiguration).GetProperties();

                foreach (var property in properties)
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    // Get the JSON property name (if it exists) or fall back to the property name
                    var jsonPropertyAttribute = property.GetCustomAttributes(typeof(JsonPropertyNameAttribute), false)
                                                        .FirstOrDefault() as JsonPropertyNameAttribute;
                    string registryValueName = jsonPropertyAttribute?.Name ?? property.Name;

                    // Get the value of the property from the DriveConfiguration object
                    object propertyValue = property.GetValue(driveConfig);

                    if (propertyValue != null)
                    {
                        // Write the value to the registry
                        subFolderKey.SetValue(registryValueName, propertyValue.ToString());
                    }
                }
            }
        }
    }
}
