// <copyright file="IBigDriveFileOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for performing file operations on BigDrive files and folders.
    /// </summary>
    /// <remarks>
    /// Provides methods to copy, move, delete, and open files and directories in BigDrive storage.
    /// </remarks>
    [ComVisible(true)]
    [Guid("7BE23F90-8D32-4D88-B4E7-59BFDA941F04")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveFileOperations
    {
        /// <summary>
        /// Copies a local file to the BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="localFilePath">The local file path to copy from.</param>
        /// <param name="bigDriveTargetPath">The destination path in BigDrive.</param>
        void CopyFileToBigDrive(
            [MarshalAs(UnmanagedType.LPStruct)] Guid driveGuid,
            [MarshalAs(UnmanagedType.LPWStr)] string localFilePath,
            [MarshalAs(UnmanagedType.LPWStr)] string bigDriveTargetPath);

        /// <summary>
        /// Copies a BigDrive file to a local storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The BigDrive file path to copy from.</param>
        /// <param name="localTargetPath">The destination path in local storage.</param>
        void CopyFileFromBigDrive(
            [MarshalAs(UnmanagedType.LPStruct)] Guid driveGuid,
            [MarshalAs(UnmanagedType.LPWStr)] string bigDriveFilePath,
            [MarshalAs(UnmanagedType.LPWStr)] string localTargetPath);

        /// <summary>
        /// Deletes a file or folder from BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to delete.</param>
        void DeleteFile(
            [MarshalAs(UnmanagedType.LPStruct)] Guid driveGuid,
            [MarshalAs(UnmanagedType.LPWStr)] string bigDriveFilePath);

        /// <summary>
        /// Creates a new directory in BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveDirectoryPath">The directory path to create.</param>
        void CreateDirectory(
            [MarshalAs(UnmanagedType.LPStruct)] Guid driveGuid,
            [MarshalAs(UnmanagedType.LPWStr)] string bigDriveDirectoryPath);

        /// <summary>
        /// Opens a BigDrive file with the associated application.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to open.</param>
        /// <param name="hwndParent">Parent window handle for any UI.</param>
        void OpenFile(
            [MarshalAs(UnmanagedType.LPStruct)] Guid driveGuid,
            [MarshalAs(UnmanagedType.LPWStr)] string bigDriveFilePath,
            IntPtr hwndParent);

        /// <summary>
        /// Moves a file within the BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="sourcePath">The source file path.</param>
        /// <param name="destinationPath">The destination file path.</param>
        void MoveFile(
            [MarshalAs(UnmanagedType.LPStruct)] Guid driveGuid,
            [MarshalAs(UnmanagedType.LPWStr)] string sourcePath,
            [MarshalAs(UnmanagedType.LPWStr)] string destinationPath);
    }
}