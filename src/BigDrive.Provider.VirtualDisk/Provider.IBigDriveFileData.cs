// <copyright file="Provider.IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileData"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public int GetFileData(Guid driveGuid, string path, out IStream stream)
        {
            stream = null;

            try
            {
                DefaultTraceSource.TraceInformation($"GetFileData: driveGuid={driveGuid}, path={path}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                Stream fileStream = client.OpenFile(NormalizePath(path));

                if (fileStream == null)
                {
                    DefaultTraceSource.TraceError("GetFileData: file not found or could not be opened");
                    return unchecked((int)0x80004005);
                }

                stream = new ComStream(fileStream);
                DefaultTraceSource.TraceInformation("GetFileData: stream created successfully");
                return 0;
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
                return unchecked((int)0x80004005);
            }
        }
    }
}
