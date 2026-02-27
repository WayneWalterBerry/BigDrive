// <copyright file="Provider.IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System;
    using System.Security.Principal;
    using System.Threading;

    using BigDrive.ConfigProvider;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveRegistration"/> for the ISO provider.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Registers the ISO provider with the BigDrive system.
        /// Only registers the provider; drives are created by the user via the shell
        /// since each drive requires an IsoFilePath configuration property.
        /// </summary>
        public void Register()
        {
            WindowsIdentity identity = WindowsIdentity.GetCurrent();

            DefaultTraceSource.TraceInformation($"Register: User={identity.Name}, ImpersonationLevel={identity.ImpersonationLevel}");

            // Register the provider only — no default drive is created.
            // Users create drives via BigDrive.Shell and set the IsoFilePath property.
            ProviderManager.RegisterProvider(ProviderConfig, CancellationToken.None);

            DefaultTraceSource.TraceInformation("Register: ISO provider registered successfully.");
        }

        /// <summary>
        /// Unregisters the ISO provider from the BigDrive system.
        /// </summary>
        public void Unregister()
        {
            DefaultTraceSource.TraceInformation("Unregister: ISO provider");

            // TODO: Implement full unregistration
            // ProviderManager.UnregisterProvider(ProviderConfig.Id, CancellationToken.None);
        }
    }
}
