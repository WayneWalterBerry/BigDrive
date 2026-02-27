// <copyright file="Provider.IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileData"/> for the ISO provider.
    /// Streams file content from ISO disc images as COM IStream.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the file data for the specified file within the ISO image.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
        /// <param name="stream">When this method returns, contains the IStream with the file data.</param>
        /// <returns>0 for success, -1 for failure.</returns>
        public int GetFileData(Guid driveGuid, string path, out IStream stream)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"GetFileData: driveGuid={driveGuid}, path={path}");

                IsoClientWrapper isoClient = GetIsoClient(driveGuid);
                Stream fileStream = isoClient.OpenFile(NormalizePath(path));

                if (fileStream == null)
                {
                    stream = null;
                    return -1;
                }

                stream = new ComStream(fileStream);
                return 0;
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
                stream = null;
                return -1;
            }
        }
    }
}
