

namespace BigDrive.ConfigProvider
{
    using BigDrive.ConfigProvider.Model;
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Text.Json.Serialization;
    using System.Threading;
    using System.Threading.Tasks;

    internal class ProviderManager
    {
        /// <summary>
        /// Writes a ProviderConfiguration to the registry under the specified subfolder name.
        /// </summary>
        /// <param name="providerConfig">The Provider object to write.</param>
        /// <param name="cancellationToken">Cancellation Token</param>
        public static void RegisterProvider(ProviderConfiguration providerConfig, CancellationToken cancellationToken)
        {
            if (providerConfig == null)
            {
                throw new ArgumentNullException(nameof(providerConfig), "Provider cannot be null.");
            }

            // Define the registry path for the specific subfolder
            string subFolderRegistryPath = $@"Software\BigDrive\Provider\{{{providerConfig.Id}}}";

            using (RegistryKey subFolderKey = Registry.CurrentUser.CreateSubKey(subFolderRegistryPath))
            {
                if (subFolderKey == null)
                {
                    throw new InvalidOperationException($"Failed to create or open registry path '{subFolderRegistryPath}'.");
                }

                // Use reflection to get all properties of the ProviderConfiguration class
                var properties = typeof(DriveConfiguration).GetProperties();

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
                        // Write the value to the registry
                        subFolderKey.SetValue(registryValueName, propertyValue.ToString());
                    }
                }
            }
        }
    }
}
