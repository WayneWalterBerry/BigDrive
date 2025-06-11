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
                .Select(child => child.Name)
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
                .Select(child => child.Name)
                .ToArray();
        }

        /// <summary>
        /// Finds a node in the tree by its path.
        /// </summary>
        /// <param name="path">The path to search for (e.g., "\RootFolder1\SubFolder1").</param>
        /// <returns>The <see cref="Node"/> at the specified path, or null if not found.</returns>
        private Node FindNodeByPath(string path)
        {
            if (string.IsNullOrEmpty(path) || path == "\\" || path == "//")
            {
                return root;
            }

            var segments = path.Trim('\\').Split(new[] { '\\' }, StringSplitOptions.RemoveEmptyEntries);
            Node current = root;
            foreach (var segment in segments)
            {
                current = current.Children.FirstOrDefault(child => child.Name == segment && child.Type == NodeType.Folder);
                if (current == null)
                {
                    return null;
                }
            }
            return current;
        }

        /// <summary>
        /// Finds a file node in the tree by its path.
        /// </summary>
        /// <param name="path">The path to search for (e.g., "\dir1\file.txt").</param>
        /// <returns>The <see cref="Node"/> representing the file at the specified path, or null if not found.</returns>
        private Node FindFileByPath(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                return null;
            }

            // Split path into segments and remove empty entries
            var segments = path.Trim('\\').Split(new[] { '\\' }, StringSplitOptions.RemoveEmptyEntries);

            if (segments.Length == 0)
            {
                return null; // No file name specified
            }

            // The file name is the last segment
            string fileName = segments[segments.Length - 1];

            // If there's only one segment (file name only), look for it at root level
            if (segments.Length == 1)
            {
                return root.Children.FirstOrDefault(child =>
                    child.Name == fileName && child.Type == NodeType.File);
            }

            // Navigate through folder hierarchy for all but the last segment
            Node currentFolder = root;
            for (int i = 0; i < segments.Length - 1; i++)
            {
                currentFolder = currentFolder.Children.FirstOrDefault(child =>
                    child.Name == segments[i] && child.Type == NodeType.Folder);

                if (currentFolder == null)
                {
                    return null; // Path not found
                }
            }

            // Find the file in the final folder
            return currentFolder.Children.FirstOrDefault(child =>
                child.Name == fileName && child.Type == NodeType.File);
        }
    }
}