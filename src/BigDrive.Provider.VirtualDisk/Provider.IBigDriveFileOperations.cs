// <copyright file="Provider.IBigDriveFileOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;
    using System.IO;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileOperations"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CopyFileToBigDrive: {localFilePath} -> {bigDriveTargetPath}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);

                using (FileStream sourceStream = File.OpenRead(localFilePath))
                {
                    client.WriteFile(NormalizePath(bigDriveTargetPath), sourceStream);
                }

                DefaultTraceSource.TraceInformation("CopyFileToBigDrive: succeeded");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CopyFileToBigDrive failed: {ex.Message}");
                throw;
            }
        }

        /// <inheritdoc/>
        public void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CopyFileFromBigDrive: {bigDriveFilePath} -> {localTargetPath}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);

                using (Stream sourceStream = client.OpenFile(NormalizePath(bigDriveFilePath)))
                using (FileStream targetStream = File.Create(localTargetPath))
                {
                    if (sourceStream == null)
                    {
                        throw new FileNotFoundException($"File not found in virtual disk: {bigDriveFilePath}");
                    }

                    sourceStream.CopyTo(targetStream);
                }

                DefaultTraceSource.TraceInformation("CopyFileFromBigDrive: succeeded");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CopyFileFromBigDrive failed: {ex.Message}");
                throw;
            }
        }

        /// <inheritdoc/>
        public void DeleteFile(Guid driveGuid, string bigDriveFilePath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"DeleteFile: {bigDriveFilePath}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                client.DeleteFile(NormalizePath(bigDriveFilePath));

                DefaultTraceSource.TraceInformation("DeleteFile: succeeded");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"DeleteFile failed: {ex.Message}");
                throw;
            }
        }

        /// <inheritdoc/>
        public void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CreateDirectory: {bigDriveDirectoryPath}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);
                client.CreateDirectory(NormalizePath(bigDriveDirectoryPath));

                DefaultTraceSource.TraceInformation("CreateDirectory: succeeded");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CreateDirectory failed: {ex.Message}");
                throw;
            }
        }

        /// <inheritdoc/>
        public void OpenFile(Guid driveGuid, string bigDriveFilePath, IntPtr hwndParent)
        {
            throw new NotSupportedException("OpenFile is not supported by the VirtualDisk provider. Use copy operations instead.");
        }

        /// <inheritdoc/>
        public void MoveFile(Guid driveGuid, string sourcePath, string destinationPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"MoveFile: {sourcePath} -> {destinationPath}");

                VirtualDiskClientWrapper client = GetClient(driveGuid);

                using (Stream sourceStream = client.OpenFile(NormalizePath(sourcePath)))
                {
                    if (sourceStream == null)
                    {
                        throw new FileNotFoundException($"Source file not found: {sourcePath}");
                    }

                    client.WriteFile(NormalizePath(destinationPath), sourceStream);
                }

                client.DeleteFile(NormalizePath(sourcePath));

                DefaultTraceSource.TraceInformation("MoveFile: succeeded");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"MoveFile failed: {ex.Message}");
                throw;
            }
        }
    }
}
