// <copyright file="Provider.IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileInfo"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public DateTime LastModifiedTime(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"LastModifiedTime: driveGuid={driveGuid}, path={path}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                return client.GetLastModifiedTime(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"LastModifiedTime failed: {ex.Message}");
                return DateTime.MinValue;
            }
        }

        /// <inheritdoc/>
        public ulong GetFileSize(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"GetFileSize: driveGuid={driveGuid}, path={path}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                return client.GetFileSize(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileSize failed: {ex.Message}");
                return 0;
            }
        }
    }
}
