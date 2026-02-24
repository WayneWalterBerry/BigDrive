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

                // Reserved property names that map to DriveConfiguration properties
                var reservedNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase) { "id", "name", "clsid" };

                // Use reflection to get all properties of the DriveConfiguration class
                var properties = typeof(DriveConfiguration).GetProperties();

                foreach (var property in properties)
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    // Skip the Properties dictionary - we handle it separately
                    if (property.Name == nameof(DriveConfiguration.Properties))
                    {
                        continue;
                    }

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

                // Read all additional properties (non-reserved values)
                foreach (string valueName in subFolderKey.GetValueNames())
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    if (!reservedNames.Contains(valueName))
                    {
                        object value = subFolderKey.GetValue(valueName);
                        if (value != null)
                        {
                            driveConfiguration.Properties[valueName] = value.ToString();
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
        /// Reads a single property value from a drive's configuration.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="propertyName">The property name to read.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>The property value, or null if not found.</returns>
        public static string ReadDriveProperty(Guid driveGuid, string propertyName, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            if (string.IsNullOrEmpty(propertyName))
            {
                throw new ArgumentNullException(nameof(propertyName), "Property name cannot be null or empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            string registryPath = $@"SOFTWARE\BigDrive\Drives\{driveGuid:B}";

            using (RegistryKey driveKey = Registry.LocalMachine.OpenSubKey(registryPath))
            {
                if (driveKey == null)
                {
                    return null;
                }

                object value = driveKey.GetValue(propertyName);
                return value?.ToString();
            }
        }

        /// <summary>
        /// Writes a single property value to a drive's configuration.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="propertyName">The property name to write.</param>
        /// <param name="value">The value to write.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        public static void WriteDriveProperty(Guid driveGuid, string propertyName, string value, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            if (string.IsNullOrEmpty(propertyName))
            {
                throw new ArgumentNullException(nameof(propertyName), "Property name cannot be null or empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            string registryPath = $@"SOFTWARE\BigDrive\Drives\{driveGuid:B}";

            using (RegistryKey driveKey = Registry.LocalMachine.OpenSubKey(registryPath, true))
            {
                if (driveKey == null)
                {
                    throw new InvalidOperationException($"Drive not found: {driveGuid}");
                }

                if (value != null)
                {
                    driveKey.SetValue(propertyName, value);
                }
                else
                {
                    driveKey.DeleteValue(propertyName, false);
                }
            }
        }

        /// <summary>
        /// Reads a secret value from Windows Credential Manager for a drive.
        /// Use this for sensitive values like API keys, OAuth tokens, and passwords.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="secretName">The secret name (e.g., "FlickrApiKey").</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>The secret value, or null if not found.</returns>
        /// <remarks>
        /// Secrets are stored in Windows Credential Manager, which is per-user.
        /// The COM+ provider must run as Interactive User to access user credentials.
        /// </remarks>
        public static string ReadSecretProperty(Guid driveGuid, string secretName, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            if (string.IsNullOrEmpty(secretName))
            {
                throw new ArgumentNullException(nameof(secretName), "Secret name cannot be null or empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            return CredentialManager.ReadSecret(driveGuid, secretName);
        }

        /// <summary>
        /// Writes a secret value to Windows Credential Manager for a drive.
        /// Use this for sensitive values like API keys, OAuth tokens, and passwords.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="secretName">The secret name (e.g., "FlickrApiKey").</param>
        /// <param name="value">The secret value to store. Pass null to delete the secret.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <remarks>
        /// Secrets are stored in Windows Credential Manager, which is per-user and encrypted.
        /// The user who stores the secret must be the same user running the COM+ provider.
        /// </remarks>
        public static void WriteSecretProperty(Guid driveGuid, string secretName, string value, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            if (string.IsNullOrEmpty(secretName))
            {
                throw new ArgumentNullException(nameof(secretName), "Secret name cannot be null or empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            CredentialManager.WriteSecret(driveGuid, secretName, value);
        }

        /// <summary>
        /// Deletes a secret from Windows Credential Manager for a drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="secretName">The secret name.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>True if the secret was deleted, false if it didn't exist.</returns>
        public static bool DeleteSecretProperty(Guid driveGuid, string secretName, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            if (string.IsNullOrEmpty(secretName))
            {
                throw new ArgumentNullException(nameof(secretName), "Secret name cannot be null or empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            return CredentialManager.DeleteSecret(driveGuid, secretName);
        }

        /// <summary>
        /// Gets all secret names for a drive from Windows Credential Manager.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>A list of secret names configured for the drive.</returns>
        public static List<string> GetSecretNames(Guid driveGuid, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            return CredentialManager.GetSecretNames(driveGuid);
        }

        /// <summary>
        /// Deletes all secrets for a drive from Windows Credential Manager.
        /// Call this when unmounting/deleting a drive to clean up credentials.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        public static void DeleteAllSecrets(Guid driveGuid, CancellationToken cancellationToken)
        {
            if (driveGuid == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveGuid), "Drive GUID cannot be empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            CredentialManager.DeleteAllSecretsForDrive(driveGuid);
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

                    // Skip the Properties dictionary - we handle it separately
                    if (property.Name == nameof(DriveConfiguration.Properties))
                    {
                        continue;
                    }

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

                // Write all custom properties
                if (driveConfig.Properties != null)
                {
                    foreach (var kvp in driveConfig.Properties)
                    {
                        cancellationToken.ThrowIfCancellationRequested();

                        if (!string.IsNullOrEmpty(kvp.Key) && kvp.Value != null)
                        {
                            subFolderKey.SetValue(kvp.Key, kvp.Value);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Deletes a drive configuration from the registry.
        /// </summary>
        /// <param name="driveId">The drive GUID to delete.</param>
        /// <param name="cancellationToken">Cancellation Token.</param>
        public static void DeleteConfiguration(Guid driveId, CancellationToken cancellationToken)
        {
            if (driveId == Guid.Empty)
            {
                throw new ArgumentNullException(nameof(driveId), "Drive ID cannot be empty.");
            }

            cancellationToken.ThrowIfCancellationRequested();

            string subFolderRegistryPath = $@"SOFTWARE\BigDrive\Drives\{{{driveId}}}";

            using (RegistryKey drivesKey = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\BigDrive\Drives", true))
            {
                if (drivesKey != null)
                {
                    drivesKey.DeleteSubKeyTree($"{{{driveId}}}", false);
                }
            }
        }

        /// <summary>
        /// Serializes a <see cref="DriveConfiguration"/> object to a JSON string.
        /// </summary>
        /// <param name="driveConfg">The <see cref="DriveConfiguration"/> instance to serialize.</param>
        /// <param name="callectionToken">A <see cref="CancellationToken"/> to observe while waiting for the task to complete.</param>
        /// <returns>A JSON-formatted string representing the <see cref="DriveConfiguration"/>.</returns>
        public static string ToJson(this DriveConfiguration driveConfg, CancellationToken callectionToken)
        {
            if (driveConfg == null)
            {
                throw new ArgumentNullException(nameof(driveConfg), "DriveConfiguration cannot be null.");
            }

            callectionToken.ThrowIfCancellationRequested();

            var options = new System.Text.Json.JsonSerializerOptions
            {
                PropertyNamingPolicy = null,
                WriteIndented = false,
                Converters = { new System.Text.Json.Serialization.JsonStringEnumConverter(System.Text.Json.JsonNamingPolicy.CamelCase) }
            };

            string json = System.Text.Json.JsonSerializer.Serialize(driveConfg, options);

            callectionToken.ThrowIfCancellationRequested();

            return json;
        }
    }
}
