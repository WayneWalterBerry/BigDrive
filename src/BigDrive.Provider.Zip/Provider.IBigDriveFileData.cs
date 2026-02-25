// <copyright file="Provider.IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileData"/> for the Zip provider.
    /// Provides file data from the ZIP archive as a COM IStream.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the file data as a stream for the file at the specified path within the ZIP archive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <param name="path">The file path (e.g., "\folder\file.txt").</param>
        /// <param name="stream">When this method returns, contains the IStream with the file data.</param>
        /// <returns>HRESULT indicating success or failure.</returns>
        public int GetFileData(Guid driveGuid, string path, [MarshalAs(UnmanagedType.Interface)] out IStream stream)
        {
            stream = null;

            try
            {
                DefaultTraceSource.TraceInformation($"GetFileData: driveGuid={driveGuid}, path={path}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);

                byte[] fileData = zipClient.GetFileData(NormalizePath(path));
                if (fileData != null && fileData.Length > 0)
                {
                    stream = new ComStream(new MemoryStream(fileData));
                    return 0; // S_OK
                }

                // E_FILENOTFOUND
                return unchecked((int)0x80070002);
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"GetFileData failed: {ex.Message}");
                // E_FAIL
                return unchecked((int)0x80004005);
            }
        }
    }
}
