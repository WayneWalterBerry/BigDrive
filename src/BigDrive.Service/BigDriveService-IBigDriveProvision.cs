// <copyright file="BigDriveService-IBigDriveProvision.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using BigDrive.ConfigProvider;
    using BigDrive.Setup;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;

    public partial class BigDriveService
    {
        public void Create(Guid driveGuid)
        {
            CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

            if (DriveManager.DriveExists(driveGuid, cancellationTokenSource.Token))
            {
                throw new InvalidOperationException($"Drive with GUID {driveGuid} already exists.");
            }

            var driveConfiguration = DriveManager.ReadConfiguration(driveGuid, cancellationTokenSource.Token);
            RegistrationHelper.RegisterShellFolder(guidDrive: driveGuid, displayName: driveConfiguration.Name, cancellationTokenSource.Token);
        }
    }
}
