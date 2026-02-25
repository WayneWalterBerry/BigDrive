// <copyright file="LocalFileStore.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.FileStores
{
    using System;
    using System.IO;

    /// <summary>
    /// IFileStore implementation backed by the local OS filesystem.
    /// </summary>
    public class LocalFileStore : IFileStore
    {
        /// <summary>
        /// The OS drive letter.
        /// </summary>
        private readonly char m_driveLetter;

        /// <summary>
        /// Initializes a new instance of the <see cref="LocalFileStore"/> class.
        /// </summary>
        /// <param name="driveLetter">The OS drive letter (e.g., 'C').</param>
        public LocalFileStore(char driveLetter)
        {
            m_driveLetter = char.ToUpper(driveLetter);
        }

        /// <summary>
        /// Gets the display name for this store.
        /// </summary>
        public string DisplayName
        {
            get { return m_driveLetter + ": (Local)"; }
        }

        /// <summary>
        /// Gets a value indicating whether this store supports file operations.
        /// </summary>
        public bool SupportsFileOperations
        {
            get { return true; }
        }

        /// <summary>
        /// Gets a value indicating whether this store supports file enumeration.
        /// </summary>
        public bool SupportsEnumeration
        {
            get { return true; }
        }

        /// <summary>
        /// Checks if a file exists at the specified path.
        /// </summary>
        /// <param name="path">The path within the drive (e.g., "\temp\file.txt").</param>
        /// <returns>True if the file exists.</returns>
        public bool FileExists(string path)
        {
            return File.Exists(ToFullPath(path));
        }

        /// <summary>
        /// Enumerates files in the specified directory.
        /// </summary>
        /// <param name="directoryPath">The directory path within the drive.</param>
        /// <returns>Array of file names.</returns>
        public string[] EnumerateFiles(string directoryPath)
        {
            string fullDir = ToFullPath(directoryPath);
            if (!Directory.Exists(fullDir))
            {
                return new string[0];
            }

            string[] fullPaths = Directory.GetFiles(fullDir);
            string[] names = new string[fullPaths.Length];
            for (int i = 0; i < fullPaths.Length; i++)
            {
                names[i] = Path.GetFileName(fullPaths[i]);
            }

            return names;
        }

        /// <summary>
        /// Enumerates subdirectories in the specified directory.
        /// </summary>
        /// <param name="directoryPath">The directory path within the drive.</param>
        /// <returns>Array of subdirectory names.</returns>
        public string[] EnumerateFolders(string directoryPath)
        {
            string fullDir = ToFullPath(directoryPath);
            if (!Directory.Exists(fullDir))
            {
                return new string[0];
            }

            string[] fullPaths = Directory.GetDirectories(fullDir);
            string[] names = new string[fullPaths.Length];
            for (int i = 0; i < fullPaths.Length; i++)
            {
                names[i] = Path.GetFileName(fullPaths[i]);
            }

            return names;
        }

        /// <summary>
        /// Copies a file from this local store to another local path.
        /// </summary>
        /// <param name="sourcePath">The source path within the drive.</param>
        /// <param name="localTempPath">The local file path to copy to.</param>
        public void CopyToLocal(string sourcePath, string localTempPath)
        {
            string fullSource = ToFullPath(sourcePath);
            ShellTrace.Verbose("LocalFileStore.CopyToLocal: \"{0}\" => \"{1}\"", fullSource, localTempPath);
            File.Copy(fullSource, localTempPath, true);
        }

        /// <summary>
        /// Copies a local file into this store.
        /// </summary>
        /// <param name="localSourcePath">The local file path to read from.</param>
        /// <param name="destinationPath">The destination path within the drive.</param>
        public void CopyFromLocal(string localSourcePath, string destinationPath)
        {
            string fullDest = ToFullPath(destinationPath);
            ShellTrace.Verbose("LocalFileStore.CopyFromLocal: \"{0}\" => \"{1}\"", localSourcePath, fullDest);

            string destDir = Path.GetDirectoryName(fullDest);
            if (!string.IsNullOrEmpty(destDir) && !Directory.Exists(destDir))
            {
                Directory.CreateDirectory(destDir);
            }

            File.Copy(localSourcePath, fullDest, true);
        }

        /// <summary>
        /// Deletes a file from this store.
        /// </summary>
        /// <param name="path">The path within the drive.</param>
        public void DeleteFile(string path)
        {
            string fullPath = ToFullPath(path);
            ShellTrace.Verbose("LocalFileStore.DeleteFile: \"{0}\"", fullPath);
            File.Delete(fullPath);
        }

        /// <summary>
        /// Moves a file within this store.
        /// </summary>
        /// <param name="sourcePath">The source path within the drive.</param>
        /// <param name="destinationPath">The destination path within the drive.</param>
        public void MoveFile(string sourcePath, string destinationPath)
        {
            string fullSource = ToFullPath(sourcePath);
            string fullDest = ToFullPath(destinationPath);
            ShellTrace.Verbose("LocalFileStore.MoveFile: \"{0}\" => \"{1}\"", fullSource, fullDest);
            File.Move(fullSource, fullDest);
        }

        /// <summary>
        /// Creates a directory in this store.
        /// </summary>
        /// <param name="directoryPath">The directory path within the drive.</param>
        public void CreateDirectory(string directoryPath)
        {
            string fullPath = ToFullPath(directoryPath);
            ShellTrace.Verbose("LocalFileStore.CreateDirectory: \"{0}\"", fullPath);
            Directory.CreateDirectory(fullPath);
        }

        /// <summary>
        /// Converts a drive-relative path to a full OS path.
        /// </summary>
        /// <param name="path">The drive-relative path (e.g., "\temp\file.txt").</param>
        /// <returns>The full OS path (e.g., "C:\temp\file.txt").</returns>
        public string ToPublicFullPath(string path)
        {
            return m_driveLetter + ":" + path;
        }

        /// <summary>
        /// Converts a drive-relative path to a full OS path.
        /// </summary>
        /// <param name="path">The drive-relative path (e.g., "\temp\file.txt").</param>
        /// <returns>The full OS path (e.g., "C:\temp\file.txt").</returns>
        private string ToFullPath(string path)
        {
            return m_driveLetter + ":" + path;
        }
    }
}
