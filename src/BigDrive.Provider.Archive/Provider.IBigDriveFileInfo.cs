// <copyright file="Provider.IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileInfo"/> for the Archive provider.
    /// Provides file metadata for entries within archive files (ZIP, TAR, 7z, RAR, etc.).
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the last modified time for the file at the specified path within the archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
        /// <returns>The last modified <see cref="DateTime"/> of the entry, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime LastModifiedTime(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"LastModifiedTime: driveGuid={driveGuid}, path={path}");

                ArchiveClientWrapper archiveClient = GetArchiveClient(driveGuid);
                return archiveClient.GetLastModifiedTime(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"LastModifiedTime failed: {ex.Message}");
                return DateTime.MinValue;
            }
        }

        /// <summary>
        /// Gets the file size for the file at the specified path within the archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
        /// <returns>The uncompressed file size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"GetFileSize: driveGuid={driveGuid}, path={path}");

                ArchiveClientWrapper archiveClient = GetArchiveClient(driveGuid);
                return archiveClient.GetFileSize(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileSize failed: {ex.Message}");
                return 0;
            }
        }
    }
}
