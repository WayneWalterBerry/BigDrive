// <copyright file="Provider.IBigDriveFileOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System;
    using System.Diagnostics;
    using System.IO;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveFileOperations"/> for the Zip provider.
    /// Provides file operations (copy, move, delete, create) for ZIP archives.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Copies a local file to the BigDrive storage (adds to ZIP archive).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="localFilePath">The local file path to copy from.</param>
        /// <param name="bigDriveTargetPath">The destination path in BigDrive.</param>
        public void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CopyFileToBigDrive: driveGuid={driveGuid}, localPath={localFilePath}, targetPath={bigDriveTargetPath}");

                if (!File.Exists(localFilePath))
                {
                    throw new FileNotFoundException("Local file not found.", localFilePath);
                }

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                string normalizedPath = NormalizePath(bigDriveTargetPath);

                zipClient.AddFile(localFilePath, normalizedPath);

                DefaultTraceSource.TraceInformation($"CopyFileToBigDrive: File added to ZIP archive: {normalizedPath}");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CopyFileToBigDrive failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Copies a BigDrive file to local storage (extracts from ZIP archive).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The BigDrive file path to copy from.</param>
        /// <param name="localTargetPath">The destination path in local storage.</param>
        public void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CopyFileFromBigDrive: driveGuid={driveGuid}, sourcePath={bigDriveFilePath}, localPath={localTargetPath}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                string normalizedPath = NormalizePath(bigDriveFilePath);

                byte[] fileData = zipClient.GetFileData(normalizedPath);
                if (fileData == null)
                {
                    throw new FileNotFoundException("File not found in ZIP archive.", bigDriveFilePath);
                }

                string targetDirectory = Path.GetDirectoryName(localTargetPath);
                if (!string.IsNullOrEmpty(targetDirectory) && !Directory.Exists(targetDirectory))
                {
                    Directory.CreateDirectory(targetDirectory);
                }

                File.WriteAllBytes(localTargetPath, fileData);

                DefaultTraceSource.TraceInformation($"CopyFileFromBigDrive: File extracted to: {localTargetPath}");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CopyFileFromBigDrive failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Deletes a file or folder from BigDrive storage (removes from ZIP archive).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to delete.</param>
        public void DeleteFile(Guid driveGuid, string bigDriveFilePath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"DeleteFile: driveGuid={driveGuid}, path={bigDriveFilePath}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                string normalizedPath = NormalizePath(bigDriveFilePath);

                zipClient.DeleteEntry(normalizedPath);

                DefaultTraceSource.TraceInformation($"DeleteFile: Entry deleted from ZIP: {normalizedPath}");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"DeleteFile failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Creates a new directory in BigDrive storage (adds directory entry to ZIP).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveDirectoryPath">The directory path to create.</param>
        public void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"CreateDirectory: driveGuid={driveGuid}, path={bigDriveDirectoryPath}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                string normalizedPath = NormalizePath(bigDriveDirectoryPath);

                zipClient.CreateDirectory(normalizedPath);

                DefaultTraceSource.TraceInformation($"CreateDirectory: Directory created in ZIP: {normalizedPath}");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"CreateDirectory failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Opens a BigDrive file with the associated application.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to open.</param>
        /// <param name="hwndParent">Parent window handle for any UI.</param>
        public void OpenFile(Guid driveGuid, string bigDriveFilePath, IntPtr hwndParent)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"OpenFile: driveGuid={driveGuid}, path={bigDriveFilePath}");

                string tempPath = Path.GetTempFileName();
                CopyFileFromBigDrive(driveGuid, bigDriveFilePath, tempPath);

                string extension = Path.GetExtension(bigDriveFilePath);
                if (!string.IsNullOrEmpty(extension))
                {
                    string tempWithExt = Path.ChangeExtension(tempPath, extension);
                    if (File.Exists(tempWithExt))
                    {
                        File.Delete(tempWithExt);
                    }

                    File.Move(tempPath, tempWithExt);
                    tempPath = tempWithExt;
                }

                Process.Start(new ProcessStartInfo(tempPath) { UseShellExecute = true });

                DefaultTraceSource.TraceInformation($"OpenFile: Opened file in default application: {tempPath}");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"OpenFile failed: {ex.Message}");
                throw;
            }
        }

        /// <summary>
        /// Moves a file within the BigDrive storage (renames/moves within ZIP).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="sourcePath">The source file path.</param>
        /// <param name="destinationPath">The destination file path.</param>
        public void MoveFile(Guid driveGuid, string sourcePath, string destinationPath)
        {
            try
            {
                DefaultTraceSource.TraceInformation($"MoveFile: driveGuid={driveGuid}, source={sourcePath}, dest={destinationPath}");

                ZipClientWrapper zipClient = GetZipClient(driveGuid);
                string normalizedSource = NormalizePath(sourcePath);
                string normalizedDest = NormalizePath(destinationPath);

                zipClient.MoveEntry(normalizedSource, normalizedDest);

                DefaultTraceSource.TraceInformation($"MoveFile: Entry moved in ZIP from {normalizedSource} to {normalizedDest}");
            }
            catch (Exception ex)
            {
                DefaultTraceSource.TraceError($"MoveFile failed: {ex.Message}");
                throw;
            }
        }
    }
}
