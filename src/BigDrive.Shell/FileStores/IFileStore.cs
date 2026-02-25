// <copyright file="IFileStore.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.FileStores
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Abstracts a file storage location (local filesystem or BigDrive provider).
    /// Commands operate on IFileStore source and destination without knowing
    /// whether the underlying storage is local or remote.
    /// </summary>
    public interface IFileStore
    {
        /// <summary>
        /// Gets a display name for this store (e.g., "C:" or "Z: (BigDrive)").
        /// </summary>
        string DisplayName { get; }

        /// <summary>
        /// Gets a value indicating whether this store supports file operations.
        /// </summary>
        bool SupportsFileOperations { get; }

        /// <summary>
        /// Gets a value indicating whether this store supports file enumeration.
        /// </summary>
        bool SupportsEnumeration { get; }

        /// <summary>
        /// Checks if a file exists at the specified path.
        /// </summary>
        /// <param name="path">The path to check.</param>
        /// <returns>True if the file exists.</returns>
        bool FileExists(string path);

        /// <summary>
        /// Enumerates files in the specified directory.
        /// </summary>
        /// <param name="directoryPath">The directory path.</param>
        /// <returns>Array of file names (not full paths).</returns>
        string[] EnumerateFiles(string directoryPath);

        /// <summary>
        /// Enumerates subdirectories in the specified directory.
        /// </summary>
        /// <param name="directoryPath">The directory path.</param>
        /// <returns>Array of subdirectory names (not full paths).</returns>
        string[] EnumerateFolders(string directoryPath);

        /// <summary>
        /// Copies a file from this store to a local temporary file.
        /// The caller is responsible for deleting the temp file.
        /// </summary>
        /// <param name="sourcePath">The source path within this store.</param>
        /// <param name="localTempPath">The local file path to write to.</param>
        void CopyToLocal(string sourcePath, string localTempPath);

        /// <summary>
        /// Copies a local file into this store at the specified destination path.
        /// </summary>
        /// <param name="localSourcePath">The local file path to read from.</param>
        /// <param name="destinationPath">The destination path within this store.</param>
        void CopyFromLocal(string localSourcePath, string destinationPath);

        /// <summary>
        /// Deletes a file at the specified path.
        /// </summary>
        /// <param name="path">The path of the file to delete.</param>
        void DeleteFile(string path);

        /// <summary>
        /// Moves a file within this store (source and destination in the same store).
        /// Not all stores support this; callers should fall back to copy+delete.
        /// </summary>
        /// <param name="sourcePath">The source path.</param>
        /// <param name="destinationPath">The destination path.</param>
        void MoveFile(string sourcePath, string destinationPath);

        /// <summary>
        /// Creates a directory at the specified path.
        /// </summary>
        /// <param name="directoryPath">The directory path to create.</param>
        void CreateDirectory(string directoryPath);
    }
}
