// <copyright file="Provider.IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    /// <summary>
    /// Implementation of <see cref="IBigDriveEnumerate"/> for the sample provider using the in-memory tree structure.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Enumerates the folder names at the specified path using the in-memory tree.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused in sample).</param>
        /// <param name="path">The path to enumerate folders under (e.g., "\RootFolder1").</param>
        /// <returns>Array of folder names at the specified path.</returns>
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            var node = FindNodeByPath(path);
            if (node == null)
            {
                return Array.Empty<string>();
            }

            return node.Children
                .Where(child => child.Type == NodeType.Folder)
                .Select(child => child.Value)
                .ToArray();
        }

        /// <summary>
        /// Enumerates the file names at the specified path using the in-memory tree.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused in sample).</param>
        /// <param name="path">The path to enumerate files under (e.g., "\RootFolder1").</param>
        /// <returns>Array of file names at the specified path.</returns>
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            var node = FindNodeByPath(path);
            if (node == null)
            {
                return Array.Empty<string>();
            }

            return node.Children
                .Where(child => child.Type == NodeType.File)
                .Select(child => child.Value)
                .ToArray();
        }

        /// <summary>
        /// Finds a node in the tree by its path.
        /// </summary>
        /// <param name="path">The path to search for (e.g., "\RootFolder1\SubFolder1").</param>
        /// <returns>The <see cref="FolderNode"/> at the specified path, or null if not found.</returns>
        private FolderNode FindNodeByPath(string path)
        {
            if (string.IsNullOrEmpty(path) || path == "\\" || path == "//")
            {
                return root;
            }

            var segments = path.Trim('\\').Split(new[] { '\\' }, StringSplitOptions.RemoveEmptyEntries);
            FolderNode current = root;
            foreach (var segment in segments)
            {
                current = current.Children.FirstOrDefault(child => child.Value == segment && child.Type == NodeType.Folder);
                if (current == null)
                {
                    return null;
                }
            }
            return current;
        }
    }
}