// <copyright file="BigDriveService-IBigDriveProvision.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.Threading;
    using BigDrive.Service;
    using global::BigDrive.ConfigProvider;

    public partial class BigDriveService
    {
        public void Create(Guid driveGuid)
        {
            DefaultTraceSource.TraceInformation("BigDriveConfiguration::GetConfiguration() called for drive: {0}", driveGuid);

            CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

            if (DriveManager.DriveExists(driveGuid, cancellationTokenSource.Token))
            {
                throw new InvalidOperationException($"Drive with GUID {driveGuid} already exists.");
            }

            var driveConfiguration = DriveManager.ReadConfiguration(driveGuid, cancellationTokenSource.Token);
            RegistryHelper.RegisterShellFolder(guidDrive: driveGuid, displayName: driveConfiguration.Name, cancellationTokenSource.Token);

            ShellHelper.RefreshMyPC(cancellationTokenSource.Token);
        }
    }
}
