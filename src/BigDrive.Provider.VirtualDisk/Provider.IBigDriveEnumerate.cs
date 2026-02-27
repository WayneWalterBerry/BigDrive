// <copyright file="Provider.IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveEnumerate"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFolders: driveGuid={driveGuid}, path={path}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                string[] folders = client.GetFolders(NormalizePath(path));

                DefaultTraceSource.TraceInformation($"EnumerateFolders: returned {folders.Length} folders");
                return folders;
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <inheritdoc/>
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFiles: driveGuid={driveGuid}, path={path}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                string[] files = client.GetFiles(NormalizePath(path));

                DefaultTraceSource.TraceInformation($"EnumerateFiles: returned {files.Length} files");
                return files;
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }
    }
}
