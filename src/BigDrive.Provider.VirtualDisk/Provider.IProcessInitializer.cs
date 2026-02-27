// <copyright file="Provider.IProcessInitializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    /// <summary>
    /// Implementation of <see cref="System.EnterpriseServices.IProcessInitializer"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Called when the COM+ application starts.
        /// </summary>
        /// <param name="punkProcessControl">Process control interface (may be null).</param>
        public void Startup(object punkProcessControl)
        {
            DefaultTraceSource.TraceInformation("VirtualDisk Provider Startup");
        }

        /// <summary>
        /// Called when the COM+ application shuts down.
        /// </summary>
        public void Shutdown()
        {
            DefaultTraceSource.TraceInformation("VirtualDisk Provider Shutdown");

            VirtualDiskClientWrapper.DisposeAll();
        }
    }
}
