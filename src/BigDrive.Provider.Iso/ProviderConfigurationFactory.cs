// <copyright file="ProviderConfigurationFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System.Reflection;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Factory for creating the provider configuration.
    /// </summary>
    internal static class ProviderConfigurationFactory
    {
        /// <summary>
        /// Creates the provider configuration for the ISO provider.
        /// </summary>
        /// <returns>A <see cref="ProviderConfiguration"/> instance.</returns>
        public static ProviderConfiguration Create()
        {
            Assembly assembly = Assembly.GetExecutingAssembly();
            AssemblyTitleAttribute titleAttribute = assembly.GetCustomAttribute<AssemblyTitleAttribute>();

            return new ProviderConfiguration
            {
                Id = Provider.CLSID,
                Name = titleAttribute?.Title ?? "BigDrive.Provider.Iso"
            };
        }
    }
}
