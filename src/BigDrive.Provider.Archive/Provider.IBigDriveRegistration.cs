// <copyright file="Provider.IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;
    using System.Security.Principal;
    using System.Threading;

    using BigDrive.ConfigProvider;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveRegistration"/> for the Archive provider.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Registers the Archive provider with the BigDrive system.
        /// Only registers the provider; drives are created by the user via the shell
        /// since each drive requires an ArchiveFilePath configuration property.
        /// </summary>
        public void Register()
        {
            WindowsIdentity identity = WindowsIdentity.GetCurrent();

            DefaultTraceSource.TraceInformation($"Register: User={identity.Name}, ImpersonationLevel={identity.ImpersonationLevel}");

            // Register the provider only — no default drive is created.
            // Users create drives via BigDrive.Shell and set the ArchiveFilePath property.
            ProviderManager.RegisterProvider(ProviderConfig, CancellationToken.None);

            DefaultTraceSource.TraceInformation("Register: Archive provider registered successfully.");
        }

        /// <summary>
        /// Unregisters the Archive provider from the BigDrive system.
        /// </summary>
        public void Unregister()
        {
            DefaultTraceSource.TraceInformation("Unregister: Archive provider");

            // TODO: Implement full unregistration
            // ProviderManager.UnregisterProvider(ProviderConfig.Id, CancellationToken.None);
        }
    }
}
