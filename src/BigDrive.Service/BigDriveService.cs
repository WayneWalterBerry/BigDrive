// <copyright file="BigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;
    using System.Threading;
    using global::BigDrive.ConfigProvider;
    using global::BigDrive.ConfigProvider.Extensions;
    using global::BigDrive.ConfigProvider.Model;
    using global::BigDrive.Service.Interfaces;

    [Guid("E6F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E7F")] // Unique GUID for the COM class
    [ClassInterface(ClassInterfaceType.None)] // No automatic interface generation
    [ComVisible(true)] // Make the class visible to COM
    public partial class BigDriveService : 
        ServicedComponent, 
        IBigDriveConfiguration,
        IBigDriveSetup,
        IBigDriveProvision
    {
        private static readonly AssemblyResolver asssemblyResolver;
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        static BigDriveService()
        {
            asssemblyResolver = AssemblyResolver.Instance;
        }

        /// <inheritdoc/>
        public string GetConfiguration(Guid guid)
        {
            DefaultTraceSource.TraceInformation("BigDriveConfiguration::GetConfiguration() called for drive: {0}", guid);

            using (CancellationTokenSource cancellationTokenSource = new CancellationTokenSource())
            {
                DriveConfiguration driveConfiguration = default(DriveConfiguration);

                try
                {
                    driveConfiguration = DriveManager.ReadConfiguration(guid, cancellationTokenSource.Token);
                }
                catch (InvalidOperationException)
                {
                    DefaultTraceSource.TraceError("BigDriveConfiguration::GetConfiguration() failed to read configuration for drive: {0}", guid);
                    throw;
                }
                catch (Exception ex)
                {
                    DefaultTraceSource.TraceError("BigDriveConfiguration::GetConfiguration() failed with exception: {0}", ex);
                    throw;
                }

                string json = driveConfiguration.ToJson();

                DefaultTraceSource.TraceInformation("BigDriveConfiguration::GetConfiguration() returned: {0}", json);

                return json;
            }
        }
    }
}
