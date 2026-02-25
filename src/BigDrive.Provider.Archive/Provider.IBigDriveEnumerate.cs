// <copyright file="Provider.IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System;
    using System.Linq;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveEnumerate"/> for the Archive provider.
    /// Enumerates folders and files within the archive file (ZIP, TAR, 7z, RAR, etc.).
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Enumerates the folder names at the specified path within the archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate folders under.</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFolders: driveGuid={driveGuid}, path={path}");

                ArchiveClientWrapper archiveClient = GetArchiveClient(driveGuid);
                return archiveClient.GetFolders(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Enumerates the file names at the specified path within the archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate files under.</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFiles: driveGuid={driveGuid}, path={path}");

                ArchiveClientWrapper archiveClient = GetArchiveClient(driveGuid);
                return archiveClient.GetFiles(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Normalizes a path by trimming leading/trailing separators and converting to forward slashes.
        /// Returns an empty string for root paths.
        /// </summary>
        /// <param name="path">The path to normalize.</param>
        /// <returns>The normalized path, or empty string for root.</returns>
        private static string NormalizePath(string path)
        {
            if (string.IsNullOrEmpty(path) || path == "\\" || path == "/" || path == "//")
            {
                return string.Empty;
            }

            return path.Trim('\\', '/').Replace('\\', '/');
        }
    }
}
