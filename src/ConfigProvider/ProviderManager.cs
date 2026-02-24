// <copyright file="ProviderManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider
{
    using BigDrive.ConfigProvider.Model;
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text.Json.Serialization;
    using System.Threading;

    /// <summary>
    /// Manages provider registrations in the Windows registry.
    /// </summary>
    public static class ProviderManager
    {
        /// <summary>
        /// The registry path for providers.
        /// </summary>
        private const string ProvidersRegistryPath = @"SOFTWARE\BigDrive\Providers";

        /// <summary>
        /// Reads all registered providers from the registry.
        /// </summary>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>Enumerable of provider configurations.</returns>
        public static IEnumerable<ProviderConfiguration> ReadProviders(CancellationToken cancellationToken)
        {
            using (RegistryKey providersKey = Registry.LocalMachine.OpenSubKey(ProvidersRegistryPath))
            {
                if (providersKey == null)
                {
                    yield break;
                }

                foreach (string subKeyName in providersKey.GetSubKeyNames())
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    ProviderConfiguration config = ReadProvider(subKeyName, cancellationToken);
                    if (config != null)
                    {
                        yield return config;
                    }
                }
            }
        }

        /// <summary>
        /// Reads a specific provider configuration from the registry.
        /// </summary>
        /// <param name="providerGuidString">The provider GUID string (with or without braces).</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>The provider configuration, or null if not found.</returns>
        public static ProviderConfiguration ReadProvider(string providerGuidString, CancellationToken cancellationToken)
        {
            string subFolderRegistryPath = $@"{ProvidersRegistryPath}\{providerGuidString}";

            using (RegistryKey subFolderKey = Registry.LocalMachine.OpenSubKey(subFolderRegistryPath))
            {
                if (subFolderKey == null)
                {
                    return null;
                }

                var config = new ProviderConfiguration();

                // Read id
                object idValue = subFolderKey.GetValue("id");
                if (idValue != null && Guid.TryParse(idValue.ToString(), out Guid id))
                {
                    config.Id = id;
                }

                // Read name
                object nameValue = subFolderKey.GetValue("name");
                if (nameValue != null)
                {
                    config.Name = nameValue.ToString();
                }

                return config;
            }
        }

        /// <summary>
        /// Writes a ProviderConfiguration to the registry under the specified subfolder name.
        /// </summary>
        /// <param name="providerConfig">The Provider object to write.</param>
        /// <param name="cancellationToken">Cancellation Token</param>
        /// <remarks>The BigDrive Shell Folder implementation reads the Software\BigDrive\Provider register subkeys to determine if the COM++ 
        /// object is installed.  These providers are called by the BigDrive Shell Folder implementation.</remarks>
        public static void RegisterProvider(ProviderConfiguration providerConfig, CancellationToken cancellationToken)
        {
            if (providerConfig == null)
            {
                throw new ArgumentNullException(nameof(providerConfig), "Provider cannot be null.");
            }

            // Define the registry path for the specific subfolder
            string subFolderRegistryPath = $@"SOFTWARE\BigDrive\Providers\{{{providerConfig.Id.ToString().ToUpper()}}}";

            // Clear the SubKey In ProviderConfiguration different properties since the last write.
            Registry.CurrentUser.DeleteSubKeyTree(subFolderRegistryPath, throwOnMissingSubKey: false);

            using (RegistryKey subFolderKey = Registry.LocalMachine.CreateSubKey(subFolderRegistryPath))
            {
                if (subFolderKey == null)
                {
                    throw new InvalidOperationException($"Failed to create or open registry path '{subFolderRegistryPath}'.");
                }

                // Use reflection to get all properties of the ProviderConfiguration class
                var properties = typeof(ProviderConfiguration).GetProperties();

                foreach (var property in properties)
                {
                    cancellationToken.ThrowIfCancellationRequested();

                    // Get the JSON property name (if it exists) or fall back to the property name
                    var jsonPropertyAttribute = property.GetCustomAttributes(typeof(JsonPropertyNameAttribute), false)
                                                        .FirstOrDefault() as JsonPropertyNameAttribute;
                    string registryValueName = jsonPropertyAttribute?.Name ?? property.Name;

                    // Get the value of the property from the ProviderConfiguration object
                    object propertyValue = property.GetValue(providerConfig);

                    if (propertyValue != null)
                    {
                        if (propertyValue is Guid guidValue)
                        {
                            // Convert the Guid to a string representation
                            propertyValue = $"{{{guidValue.ToString().ToUpper()}}}";
                        }

                        // Write the value to the registry
                        subFolderKey.SetValue(registryValueName, propertyValue.ToString());
                    }
                }
            }
        }

        /// <summary>
        /// Reads unregisters the provider.
        /// </summary>
        /// <param name="guidProvider">Provider Guid</param>
        /// <param name="cancellationToken">Cancellation Token</param>
        public static void UnRegisterProvider(Guid guidProvider, CancellationToken cancellationToken)
        {
            // Define the registry path for the specific subfolder
            string subFolderRegistryPath = $@"SOFTWARE\BigDrive\Providers\{{{guidProvider.ToString().ToUpper()}}}";
            using (RegistryKey subFolderKey = Registry.LocalMachine.OpenSubKey(subFolderRegistryPath, true))
            {
                if (subFolderKey != null)
                {
                    // Delete the subfolder from the registry
                    Registry.LocalMachine.DeleteSubKeyTree(subFolderRegistryPath, false);
                }
            }
        }
    }
}
