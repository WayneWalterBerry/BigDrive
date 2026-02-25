// <copyright file="Provider.IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileInfo"/> for the Zip provider.
    /// Provides file metadata for entries within the ZIP archive.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the last modified time for the file at the specified path within the ZIP archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
        /// <returns>The last modified <see cref="DateTime"/> of the entry, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime LastModifiedTime(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"LastModifiedTime: driveGuid={driveGuid}, path={path}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                return zipClient.GetLastModifiedTime(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"LastModifiedTime failed: {ex.Message}");
                return DateTime.MinValue;
            }
        }

        /// <summary>
        /// Gets the file size for the file at the specified path within the ZIP archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
        /// <returns>The uncompressed file size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"GetFileSize: driveGuid={driveGuid}, path={path}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                return zipClient.GetFileSize(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileSize failed: {ex.Message}");
                return 0;
            }
        }
    }
}
