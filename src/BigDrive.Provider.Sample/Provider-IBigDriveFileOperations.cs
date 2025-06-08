// <copyright file="Provider.IBigDriveFileOperations.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.IO;
    using System.Diagnostics;

    /// <summary>
    /// Implements file operations for the sample BigDrive provider using the in-memory root structure.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Copies a local file to the BigDrive storage (adds a file node to the tree).
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="localFilePath">The local file path to copy from.</param>
        /// <param name="bigDriveTargetPath">The destination path in BigDrive.</param>
        public void CopyFileToBigDrive(Guid driveGuid, string localFilePath, string bigDriveTargetPath)
        {
            System.Diagnostics.Debugger.Launch();

            if (!File.Exists(localFilePath))
            {
                throw new FileNotFoundException("Local file not found.", localFilePath);
            }

            var parentPath = Path.GetDirectoryName(bigDriveTargetPath)?.Replace(Path.DirectorySeparatorChar, '\\') ?? "\\";
            var fileName = Path.GetFileName(bigDriveTargetPath);
            var parentNode = FindNodeByPath(parentPath);

            if (parentNode == null || parentNode.Type != NodeType.Folder)
            {
                throw new DirectoryNotFoundException("Target directory not found in BigDrive: " + parentPath);
            }

            var fileInfo = new FileInfo(localFilePath);

            var newNode = new FolderNode(fileInfo.Name)
            {
                Type = NodeType.File,
                LastModifiedDate = fileInfo.LastWriteTime,
                Size = (ulong)fileInfo.Length
            };

            parentNode.Children.Add(newNode);
        }

        /// <summary>
        /// Copies a BigDrive file to a local storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The BigDrive file path to copy from.</param>
        /// <param name="localTargetPath">The destination path in local storage.</param>
        public void CopyFileFromBigDrive(Guid driveGuid, string bigDriveFilePath, string localTargetPath)
        {
            var node = FindNodeByPath(bigDriveFilePath);
            if (node == null || node.Type != NodeType.File)
            {
                throw new FileNotFoundException("BigDrive file not found.", bigDriveFilePath);
            }

            // For the sample, just create an empty file with the correct size
            using (var fs = new FileStream(localTargetPath, FileMode.Create, FileAccess.Write))
            {
                fs.SetLength((long)node.Size);
            }
            File.SetLastWriteTime(localTargetPath, node.LastModifiedDate);
        }

        /// <summary>
        /// Deletes a file or folder from BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to delete.</param>
        public void DeleteFile(Guid driveGuid, string bigDriveFilePath)
        {
            var parentPath = Path.GetDirectoryName(bigDriveFilePath)?.Replace(Path.DirectorySeparatorChar, '\\') ?? "\\";
            var name = Path.GetFileName(bigDriveFilePath);

            var parentNode = FindNodeByPath(parentPath);
            if (parentNode == null)
            {
                throw new DirectoryNotFoundException("Parent directory not found in BigDrive: " + parentPath);
            }

            var node = parentNode.Children.Find(child => child.Name == name);
            if (node == null)
            {
                throw new FileNotFoundException("File or folder not found in BigDrive: " + bigDriveFilePath);
            }

            parentNode.Children.Remove(node);
        }

        /// <summary>
        /// Creates a new directory in BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveDirectoryPath">The directory path to create.</param>
        public void CreateDirectory(Guid driveGuid, string bigDriveDirectoryPath)
        {
            var parentPath = Path.GetDirectoryName(bigDriveDirectoryPath)?.Replace(Path.DirectorySeparatorChar, '\\') ?? "\\";
            var dirName = Path.GetFileName(bigDriveDirectoryPath);
            var parentNode = FindNodeByPath(parentPath);

            if (parentNode == null || parentNode.Type != NodeType.Folder)
            {
                throw new DirectoryNotFoundException("Parent directory not found in BigDrive: " + parentPath);
            }

            if (parentNode.Children.Exists(child => child.Name == dirName && child.Type == NodeType.Folder))
            {
                throw new IOException("Directory already exists: " + bigDriveDirectoryPath);
            }

            parentNode.Children.Add(new FolderNode(dirName) { Type = NodeType.Folder });
        }

        /// <summary>
        /// Opens a BigDrive file with the associated application.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="bigDriveFilePath">The file path to open.</param>
        /// <param name="hwndParent">Parent window handle for any UI.</param>
        public void OpenFile(Guid driveGuid, string bigDriveFilePath, IntPtr hwndParent)
        {
            var tempPath = Path.GetTempFileName();
            CopyFileFromBigDrive(driveGuid, bigDriveFilePath, tempPath);
            Process.Start(new ProcessStartInfo(tempPath) { UseShellExecute = true });
        }

        /// <summary>
        /// Moves a file within the BigDrive storage.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="sourcePath">The source file path.</param>
        /// <param name="destinationPath">The destination file path.</param>
        public void MoveFile(Guid driveGuid, string sourcePath, string destinationPath)
        {
            var srcParentPath = Path.GetDirectoryName(sourcePath)?.Replace(Path.DirectorySeparatorChar, '\\') ?? "\\";
            var srcName = Path.GetFileName(sourcePath);
            var srcParentNode = FindNodeByPath(srcParentPath);
            if (srcParentNode == null)
            {
                throw new DirectoryNotFoundException("Source parent directory not found: " + srcParentPath);
            }

            var node = srcParentNode.Children.Find(child => child.Name == srcName);
            if (node == null)
            {
                throw new FileNotFoundException("Source file or folder not found: " + sourcePath);
            }

            var destParentPath = Path.GetDirectoryName(destinationPath)?.Replace(Path.DirectorySeparatorChar, '\\') ?? "\\";
            var destName = Path.GetFileName(destinationPath);
            var destParentNode = FindNodeByPath(destParentPath);

            if (destParentNode == null || destParentNode.Type != NodeType.Folder)
            {
                throw new DirectoryNotFoundException("Destination parent directory not found: " + destParentPath);
            }

            if (destParentNode.Children.Exists(child => child.Name == destName))
            {
                throw new IOException("Destination already exists: " + destinationPath);
            }

            srcParentNode.Children.Remove(node);
            node.Name = destName;
            destParentNode.Children.Add(node);
        }
    }
}