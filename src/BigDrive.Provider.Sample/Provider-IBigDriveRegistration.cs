// <copyright file="Provider.IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using BigDrive.ConfigProvider;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Security.Principal;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;

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
        }

        /// <inheritdoc/>
        public void Unregister()
        {
            // TODO
        }
    }
}
