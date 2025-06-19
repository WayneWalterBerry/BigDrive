// <copyright file="BigDriveService-IBigDriveProvision.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service
{
    using System;
    using System.Threading;
    using BigDrive.Service;
    using global::BigDrive.ConfigProvider;
    using global::BigDrive.ConfigProvider.Model;

    public partial class BigDriveService
    {
        public void Create(Guid driveGuid)
        {
            DefaultTraceSource.TraceInformation("IBigDriveProvision::Create() called for drive: {0}", driveGuid);

            CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

            if (DriveManager.DriveExists(driveGuid, cancellationTokenSource.Token))
            {
                throw new InvalidOperationException($"Drive with GUID {driveGuid} already exists.");
            }

            var driveConfiguration = DriveManager.ReadConfiguration(driveGuid, cancellationTokenSource.Token);
            RegistryHelper.RegisterShellFolder(guidDrive: driveGuid, displayName: driveConfiguration.Name, cancellationTokenSource.Token);

            ShellHelper.RefreshMyPC(cancellationTokenSource.Token);
        }

        /// <summary>
        /// Creates a new drive configuration from the specified JSON configuration string.
        /// </summary>
        /// <remarks>This method parses the provided JSON configuration, validates it, and creates a new
        /// drive configuration. The configuration is written to the registry, and the shell folder for the drive is
        /// registered. If the operation is successful, the system's "My PC" view is refreshed to reflect the new
        /// drive.</remarks>
        /// <param name="jsonConfiguration">A JSON-formatted string representing the drive configuration. The string must contain valid configuration
        /// data, including a unique GUID and other required properties.</param>
        /// <exception cref="ArgumentException">Thrown if <paramref name="jsonConfiguration"/> is null, empty, or contains invalid JSON data.</exception>
        /// <exception cref="InvalidOperationException">Thrown if a drive with the GUID specified in the configuration already exists.</exception>
        public void CreateFromConfiguration(string jsonConfiguration)
        {
            System.Diagnostics.Debugger.Launch();

            DefaultTraceSource.TraceInformation("IBigDriveProvision::GetConfiguration() called for configuration: {0}", jsonConfiguration);
            CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();

            // Parse the JSON configuration to create a DriveConfiguration object
            DriveConfiguration driveConfiguration = DriveManager.ReadConfigurationFromJson(jsonConfiguration, cancellationTokenSource.Token);
            if (driveConfiguration == null)
            {
                throw new ArgumentException("Invalid JSON configuration provided.", nameof(jsonConfiguration));
            }

            // Check that the drive does not already exist.
            if (DriveManager.DriveExists(driveConfiguration.Id, cancellationTokenSource.Token))
            {
                throw new InvalidOperationException($"Drive with GUID {driveConfiguration.Id} already exists.");
            }

            // Write the drive configuration to the registry
            DriveManager.WriteConfiguration(driveConfiguration, cancellationTokenSource.Token);

            // Register the shell folder for the drive
            RegistryHelper.RegisterShellFolder(guidDrive: driveConfiguration.Id, displayName: driveConfiguration.Name, cancellationTokenSource.Token);

            ShellHelper.RefreshMyPC(cancellationTokenSource.Token);
        }
    }
}
