// <copyright file="FileTransferService.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.FileStores
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;

    /// <summary>
    /// Performs file transfer operations between any two IFileStore instances.
    /// Eliminates the need for separate Local-to-BigDrive, BigDrive-to-Local, etc. methods.
    /// Supports copy, move (copy + delete source), and wildcard patterns.
    /// </summary>
    public static class FileTransferService
    {
        /// <summary>
        /// Copies a single file from source store to destination store.
        /// Uses a local temp file as intermediary when stores differ.
        /// </summary>
        /// <param name="source">The source file store.</param>
        /// <param name="sourcePath">The source file path within the store.</param>
        /// <param name="destination">The destination file store.</param>
        /// <param name="destinationPath">The destination file path within the store.</param>
        public static void CopyFile(IFileStore source, string sourcePath, IFileStore destination, string destinationPath)
        {
            ShellTrace.Info("FileTransferService.CopyFile: [{0}] \"{1}\" => [{2}] \"{3}\"",
                source.DisplayName, sourcePath, destination.DisplayName, destinationPath);

            // Same store type optimization: if both are local, use direct copy
            if (source is LocalFileStore && destination is LocalFileStore)
            {
                source.CopyToLocal(sourcePath, ((LocalFileStore)destination).ToPublicFullPath(destinationPath));
                return;
            }

            // General case: source → temp file → destination
            if (source is LocalFileStore)
            {
                // Local to BigDrive: no temp file needed, local file already has correct name
                string localFullPath = ((LocalFileStore)source).ToPublicFullPath(sourcePath);
                destination.CopyFromLocal(localFullPath, destinationPath);
            }
            else if (destination is LocalFileStore)
            {
                // BigDrive to Local: no temp file needed
                string localFullPath = ((LocalFileStore)destination).ToPublicFullPath(destinationPath);
                source.CopyToLocal(sourcePath, localFullPath);
            }
            else
            {
                // BigDrive to BigDrive (same or cross-provider): use temp file as intermediary.
                // Preserve the target filename in the temp path so providers that use
                // the local filename get the correct name.
                CopyViaTempFile(source, sourcePath, destination, destinationPath);
            }
        }

        /// <summary>
        /// Moves a single file from source store to destination store.
        /// If both are the same BigDrive, uses the native MoveFile.
        /// Otherwise, performs copy + delete source.
        /// </summary>
        /// <param name="source">The source file store.</param>
        /// <param name="sourcePath">The source file path within the store.</param>
        /// <param name="destination">The destination file store.</param>
        /// <param name="destinationPath">The destination file path within the store.</param>
        public static void MoveFile(IFileStore source, string sourcePath, IFileStore destination, string destinationPath)
        {
            ShellTrace.Info("FileTransferService.MoveFile: [{0}] \"{1}\" => [{2}] \"{3}\"",
                source.DisplayName, sourcePath, destination.DisplayName, destinationPath);

            // Same BigDrive: use native MoveFile
            if (source is BigDriveFileStore && destination is BigDriveFileStore)
            {
                BigDriveFileStore srcBd = (BigDriveFileStore)source;
                BigDriveFileStore dstBd = (BigDriveFileStore)destination;

                if (srcBd.DriveGuid == dstBd.DriveGuid)
                {
                    srcBd.MoveFile(sourcePath, destinationPath);
                    return;
                }
            }

            // Same local drive: use native File.Move
            if (source is LocalFileStore && destination is LocalFileStore)
            {
                source.MoveFile(sourcePath, destinationPath);
                return;
            }

            // Cross-store move: copy then delete source
            CopyFile(source, sourcePath, destination, destinationPath);
            source.DeleteFile(sourcePath);
            ShellTrace.Info("Deleted source after move: \"{0}\"", sourcePath);
        }

        /// <summary>
        /// Resolves the destination file path, appending the source filename when
        /// the destination appears to be a directory (matches cmd.exe behavior).
        /// </summary>
        /// <param name="sourceFilePath">The source file path.</param>
        /// <param name="destinationPath">The destination path (may be a directory).</param>
        /// <returns>The resolved destination file path.</returns>
        public static string ResolveDestinationFilePath(string sourceFilePath, string destinationPath)
        {
            string sourceFileName = sourceFilePath.Substring(sourceFilePath.LastIndexOf('\\') + 1);

            // If destination already ends with a filename that has an extension, use as-is
            if (destinationPath.LastIndexOf('.') > destinationPath.LastIndexOf('\\'))
            {
                return destinationPath;
            }

            // If destination already ends with the source filename, use as-is
            if (destinationPath.EndsWith(sourceFileName, StringComparison.OrdinalIgnoreCase))
            {
                return destinationPath;
            }

            // Destination is a directory: append source filename
            return CombinePath(destinationPath, sourceFileName);
        }

        /// <summary>
        /// Combines two path segments.
        /// </summary>
        /// <param name="basePath">The base path.</param>
        /// <param name="fileName">The file name to append.</param>
        /// <returns>The combined path.</returns>
        public static string CombinePath(string basePath, string fileName)
        {
            if (string.IsNullOrEmpty(basePath) || basePath == "\\")
            {
                return "\\" + fileName;
            }

            return basePath.TrimEnd('\\') + "\\" + fileName;
        }

        /// <summary>
        /// Copies a file via a local temp file, preserving the target filename.
        /// Creates a unique temp directory so the local file sent to the provider
        /// has the correct filename (some providers use the local filename).
        /// </summary>
        /// <param name="source">The source file store.</param>
        /// <param name="sourcePath">The source path within the store.</param>
        /// <param name="destination">The destination file store.</param>
        /// <param name="destinationPath">The destination path within the store.</param>
        private static void CopyViaTempFile(IFileStore source, string sourcePath, IFileStore destination, string destinationPath)
        {
            // Extract the target filename from the destination path
            string targetFileName = destinationPath.Substring(destinationPath.LastIndexOf('\\') + 1);

            // Create a unique temp directory so the file keeps its correct name
            string tempDir = Path.Combine(Path.GetTempPath(), "BigDrive_" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(tempDir);

            string tempFile = Path.Combine(tempDir, targetFileName);
            ShellTrace.Verbose("Temp file with preserved name: \"{0}\"", tempFile);

            try
            {
                source.CopyToLocal(sourcePath, tempFile);
                destination.CopyFromLocal(tempFile, destinationPath);
            }
            finally
            {
                if (File.Exists(tempFile))
                {
                    File.Delete(tempFile);
                }

                if (Directory.Exists(tempDir))
                {
                    Directory.Delete(tempDir);
                }
            }
        }
    }
}
