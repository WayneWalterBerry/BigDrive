// <copyright file="ProviderConfigurationFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Factory for creating VirtualDisk provider configuration.
    /// </summary>
    internal static class ProviderConfigurationFactory
    {
        /// <summary>
        /// Creates the provider configuration for registry registration.
        /// </summary>
        /// <returns>Provider configuration instance.</returns>
        public static ProviderConfiguration Create()
        {
            return new ProviderConfiguration
            {
                Id = Provider.CLSID,
                Name = "VirtualDisk"
            };
        }
    }
}
