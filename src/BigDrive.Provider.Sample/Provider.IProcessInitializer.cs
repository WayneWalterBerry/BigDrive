// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Implementation of <see cref="System.EnterpriseServices.IProcessInitializer"/> for the Sample provider.
    /// Handles COM+ application startup and shutdown.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Lazy-loaded provider configuration to ensure AssemblyResolver runs first.
        /// </summary>
        private static readonly Lazy<ProviderConfiguration> _providerConfiguration = 
            new Lazy<ProviderConfiguration>(() => ProviderConfigurationFactory.Create());

        /// <summary>
        /// Gets the provider configuration.
        /// </summary>
        private static ProviderConfiguration ProviderConfig => _providerConfiguration.Value;

        /// <summary>
        /// This method is called when the COM++ Service starts the provider.
        /// </summary>
        /// <param name="punkProcessControl">
        /// In Microsoft Windows XP, a pointer to the IUnknown interface of the COM component starting up.
        /// In Microsoft Windows 2000, this argument is always null.
        /// </param>
        public void Startup(object punkProcessControl)
        {
        }

        /// <summary>
        /// This method is called when the COM++ Service shutdown the provider.
        /// </summary>
        public void Shutdown()
        {
        }
    }
}
