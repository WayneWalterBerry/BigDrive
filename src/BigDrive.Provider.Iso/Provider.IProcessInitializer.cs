// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System.EnterpriseServices;

    /// <summary>
    /// Implements IProcessInitializer for COM+ lifecycle management.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public void Startup(object punkProcessControl)
        {
            DefaultTraceSource.TraceInformation("Provider.Startup: ISO provider starting up.");
        }

        /// <inheritdoc/>
        public void Shutdown()
        {
            DefaultTraceSource.TraceInformation("Provider.Shutdown: ISO provider shutting down.");
        }
    }
}
