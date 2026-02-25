// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Implementation of <see cref="System.EnterpriseServices.IProcessInitializer"/> for the Archive provider.
    /// Handles COM+ application startup and shutdown.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Static constructor ensures AssemblyResolver is initialized before any other code.
        /// </summary>
        static Provider()
        {
            AssemblyResolver.Initialize();
        }

        /// <summary>
        /// Called when the COM+ Service starts the provider.
        /// </summary>
        /// <param name="punkProcessControl">
        /// In Windows XP, a pointer to the IUnknown interface of the COM component starting up.
        /// In Windows 2000, this argument is always null.
        /// </param>
        public void Startup(object punkProcessControl)
        {
            DefaultTraceSource.TraceInformation("Archive Provider Startup");
        }

        /// <summary>
        /// Called when the COM+ Service shuts down the provider.
        /// </summary>
        public void Shutdown()
        {
            DefaultTraceSource.TraceInformation("Archive Provider Shutdown");
        }
    }
}
