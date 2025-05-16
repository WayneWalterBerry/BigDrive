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

    public partial class Provider
    {
        /// <inheritdoc/>
        public void Register()
        {
            // The Expectation is that the Current User Have Admin Permissions to the Registry.
            WindowsIdentity identity = WindowsIdentity.GetCurrent();

            DefaultTraceSource.TraceInformation($"User: {identity.Name}");
            DefaultTraceSource.TraceInformation($"Impersonation Level: {identity.ImpersonationLevel}");

            ProviderManager.RegisterProvider(providerConfiguration, CancellationToken.None);

            // Most Providers Will Not Create Their Own Drive, However This One Doe For Testing Purposes.
            {
                DriveConfiguration driveConfiguration = new DriveConfiguration
                {
                    CLSID = providerConfiguration.Id,
                    Name = providerConfiguration.Name,
                    Id = Guid.Parse("6369DDE1-9A63-4E3B-B3C0-62A8082ED32E")
                };

                DriveManager.WriteConfiguration(driveConfiguration, CancellationToken.None);
            }
        }

        /// <inheritdoc/>
        public void Unregister()
        {
            // TODO
        }
    }
}
