// <copyright file="Provider.IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System;
    using System.Linq;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveEnumerate"/> for the ISO provider.
    /// Enumerates folders and files within the ISO disc image.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Enumerates the folder names at the specified path within the ISO image.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate folders under.</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFolders: driveGuid={driveGuid}, path={path}");

                IsoClientWrapper isoClient = GetIsoClient(driveGuid);
                return isoClient.GetFolders(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }

        /// <summary>
        /// Enumerates the file names at the specified path within the ISO image.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The path to enumerate files under.</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"EnumerateFiles: driveGuid={driveGuid}, path={path}");

                IsoClientWrapper isoClient = GetIsoClient(driveGuid);
                return isoClient.GetFiles(NormalizePath(path));
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"EnumerateFiles failed: {ex.Message}");
                return Array.Empty<string>();
            }
        }
    }
}
