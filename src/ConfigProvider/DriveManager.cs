// <copyright file="DriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider
{
    using BigDrive.ConfigProvider.Model;
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text.Json;
    using System.Text.Json.Serialization;
    using System.Threading;

    public static class DriveManager
    {
        /// <summary>
        /// Checks if a drive with the specified Guid exists in the registry under SOFTWARE\BigDrive\Drives.
        /// </summary>
        /// <param name="driveId">The unique identifier (Guid) of the drive.</param>
        /// <param name="cancellationToken">Cancellation Token</param>
        /// <returns>True if the drive exists; otherwise, false.</returns>
        public static bool DriveExists(Guid guidDrive, CancellationToken cancellationToken)
        {
            if (guidDrive == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(guidDrive), "Guid cannot be empty.");
            }

            string subFolderRegistryPath = $@"SOFTWARE\BigDrive\Drives\{guidDrive:B}";
            using (RegistryKey subFolderKey = Registry.LocalMachine.OpenSubKey(subFolderRegistryPath))
            {
                return subFolderKey != null;
            }
        }

        /// <summary>
        /// Reads the Drives subfolder in the registry and iterates all subfolders, calling LoadConfiguration for each.
        /// </summary>
        public static IEnumerable<DriveConfiguration> ReadConfigurations(CancellationToken cancellationToken)
        {
            string drivesRegistryPath = Path.Combine("SOFTWARE\\BigDrive", "Drives");

            using (RegistryKey drivesKey = Registry.LocalMachine.OpenSubKey(@"Software\BigDrive\Drives"))
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
        public static DriveConfiguration ReadConfiguration(Guid guidDrive, CancellationToken cancellationToken)
        {
            if (guidDrive== Guid.Empty)
            {
                throw new ArgumentNullException(nameof(guidDrive), "Guid cannot be empty.");
            }

            // Define the registry path for the specific subfolder
            string subFolderRegistryPath = $@"SOFTWARE\BigDrive\Drives\{guidDrive:B}";

            using (RegistryKey subFolderKey = Registry.LocalMachine.OpenSubKey(subFolderRegistryPath))
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
                if (guidDrive != driveConfiguration.Id)
                {
                    throw new InvalidOperationException($"Subfolder name of register key: '{subFolderKey.Name}' does not match Id found in that key '{driveConfiguration.Id}'.");
                }

                // Return the populated DriveConfiguration object
                return driveConfiguration;
            }
        }

        /// <summary>
        /// Deserializes a JSON string into a <see cref="DriveConfiguration"/> object.
        /// </summary>
        /// <remarks>The method uses case-insensitive property name matching and supports enum values
        /// serialized as camel-case strings. If the JSON string is invalid or does not match the expected structure of
        /// <see cref="DriveConfiguration"/>, the method returns <see langword="null"/> instead of throwing an
        /// exception.</remarks>
        /// <param name="jsonConfiguration">A JSON-formatted string representing the configuration. Cannot be null, empty, or whitespace.</param>
        /// <param name="cancellationToken">A token that can be used to cancel the operation. If cancellation is requested, the method will throw <see
        /// cref="OperationCanceledException"/>.</param>
        /// <returns>A <see cref="DriveConfiguration"/> object deserialized from the provided JSON string, or <see
        /// langword="null"/> if the JSON is invalid or cannot be deserialized.</returns>
        /// <exception cref="ArgumentNullException">Thrown if <paramref name="jsonConfiguration"/> is null, empty, or consists only of whitespace.</exception>
        public static DriveConfiguration ReadConfigurationFromJson(string jsonConfiguration, CancellationToken cancellationToken)
        {
            if (string.IsNullOrWhiteSpace(jsonConfiguration))
            {
                throw new ArgumentNullException(nameof(jsonConfiguration), "JSON configuration cannot be null or empty.");
            }

            var options = new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true,
                Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase) }
            };

            // Support cancellation
            cancellationToken.ThrowIfCancellationRequested();

            DriveConfiguration config;
            try
            {
                config = JsonSerializer.Deserialize<DriveConfiguration>(jsonConfiguration, options);
            }
            catch (JsonException)
            {
                return null;
            }

            cancellationToken.ThrowIfCancellationRequested();

            return config;
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
            string subFolderRegistryPath = $@"SOFTWARE\BigDrive\Drives\{{{driveConfig.Id}}}";

            using (RegistryKey subFolderKey = Registry.LocalMachine.CreateSubKey(subFolderRegistryPath))
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
