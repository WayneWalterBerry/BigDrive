// <copyright file="Provider.IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Security.Principal;
    using System.Threading;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveRegistration"/> for the Sample provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public void Register()
        {
            WindowsIdentity identity = WindowsIdentity.GetCurrent();

            DefaultTraceSource.TraceInformation($"Register: User={identity.Name}, ImpersonationLevel={identity.ImpersonationLevel}");

            ProviderManager.RegisterProvider(ProviderConfig, CancellationToken.None);

            // Most Providers Will Not Create Their Own Drive, However This One Does For Testing Purposes.
            DriveConfiguration driveConfiguration = new DriveConfiguration
            {
                CLSID = ProviderConfig.Id,
                Name = ProviderConfig.Name,
                Id = Guid.Parse("6369DDE1-9A63-4E3B-B3C0-62A8082ED32E")
            };

            DriveManager.WriteConfiguration(driveConfiguration, CancellationToken.None);

            DefaultTraceSource.TraceInformation("Register: Sample provider registered successfully.");
        }

        /// <inheritdoc/>
        public void Unregister()
        {
            DefaultTraceSource.TraceInformation("Unregister: Sample provider");

            // TODO: Implement full unregistration
            // DriveManager.DeleteConfiguration(Guid.Parse("6369DDE1-9A63-4E3B-B3C0-62A8082ED32E"), CancellationToken.None);
            // ProviderManager.UnregisterProvider(ProviderConfig.Id, CancellationToken.None);
        }
    }
}
