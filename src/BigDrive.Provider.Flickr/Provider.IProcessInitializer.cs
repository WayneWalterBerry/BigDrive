// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Implementation of <see cref="System.EnterpriseServices.IProcessInitializer"/> for the Flickr provider.
    /// Handles COM+ application startup and shutdown.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// The provider configuration.
        /// </summary>
        private static readonly ProviderConfiguration providerConfiguration = ProviderConfigurationFactory.Create();

        /// <summary>
        /// Called when the COM+ Service starts the provider.
        /// </summary>
        /// <param name="punkProcessControl">
        /// In Windows XP, a pointer to the IUnknown interface of the COM component starting up.
        /// In Windows 2000, this argument is always null.
        /// </param>
        public void Startup(object punkProcessControl)
        {
            DefaultTraceSource.TraceInformation("Flickr Provider Startup");

            // Initialize Flickr client if needed
            // FlickrClient.Initialize() could be called here if authentication is required at startup
        }

        /// <summary>
        /// Called when the COM+ Service shuts down the provider.
        /// </summary>
        public void Shutdown()
        {
            DefaultTraceSource.TraceInformation("Flickr Provider Shutdown");
        }
    }
}
