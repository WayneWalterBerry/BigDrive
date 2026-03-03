// <copyright file="Provider.IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;
    using System.Security.Principal;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveRegistration"/> for the Flickr provider.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Registers the Flickr provider with the BigDrive system.
        /// </summary>
        public void Register()
        {
            WindowsIdentity identity = WindowsIdentity.GetCurrent();

            DefaultTraceSource.TraceInformation($"Register: User={identity.Name}, ImpersonationLevel={identity.ImpersonationLevel}");

            // Register the provider
            ProviderManager.RegisterProvider(ProviderConfig, CancellationToken.None);

            DefaultTraceSource.TraceInformation("Register: Flickr provider registered successfully.");
        }

        /// <summary>
        /// Unregisters the Flickr provider from the BigDrive system.
        /// </summary>
        public void Unregister()
        {
            DefaultTraceSource.TraceInformation("Unregister: Flickr provider");

            // TODO: Implement full unregistration
            // DriveManager.DeleteConfiguration(FlickrDriveId, CancellationToken.None);
            // ProviderManager.UnregisterProvider(ProviderConfig.Id, CancellationToken.None);
        }
    }
}
