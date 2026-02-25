// <copyright file="IBigDriveProvision.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for provisioning and deprovisioning BigDrive virtual drives.
    /// Implemented by BigDrive.Service, which runs in COM+ with elevated registry access.
    /// Called by BigDrive.Shell and other clients via COM+ out-of-process activation.
    /// </summary>
    /// <remarks>
    /// The Shell must never write to the registry directly. All drive creation and removal
    /// operations go through this interface, which is implemented by BigDrive.Service
    /// running in dllhost.exe with the necessary permissions.
    /// </remarks>
    [ComVisible(true)]
    [Guid("293D4995-FDFB-46FD-A0C6-A7DE2DA5B13F")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveProvision
    {
        /// <summary>
        /// Mounts a new drive with the specified GUID.
        /// </summary>
        /// <param name="driveGuid">The GUID of the drive to mount.</param>
        void Mount(Guid driveGuid);

        /// <summary>
        /// Mounts a new drive from the specified JSON configuration.
        /// </summary>
        /// <param name="jsonConfiguration">A JSON-formatted string representing the drive configuration.</param>
        void Mount(string jsonConfiguration);

        /// <summary>
        /// Unmounts a drive by removing its configuration from the registry,
        /// unregistering its shell folder, and refreshing Explorer.
        /// </summary>
        /// <param name="driveGuid">The GUID of the drive to unmount.</param>
        void UnmountDrive(Guid driveGuid);
    }
}
