// <copyright file="Provider.IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;

    /// <summary>
    /// Provides file information using the in-memory root structure.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the last modified time for the file at the specified path using the root structure.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused in sample).</param>
        /// <param name="path">The file path (e.g., "\A File.txt").</param>
        /// <returns>The last modified <see cref="DateTime"/> of the file, or <see cref="DateTime.MinValue"/> if not found.</returns>
        public DateTime LastModifiedTime(Guid driveGuid, string path)
        {
            var node = FindNodeByPath(path);
            if (node != null && node.Type == NodeType.File)
            {
                return node.LastModifiedDate;
            }
            return DateTime.MinValue;
        }

        /// <summary>
        /// Gets the file size for the file at the specified path using the root structure.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused in sample).</param>
        /// <param name="path">The file path (e.g., "\A File.txt").</param>
        /// <returns>The file size in bytes, or 0 if not found.</returns>
        public ulong GetFileSize(Guid driveGuid, string path)
        {
            var node = FindNodeByPath(path);
            if (node != null && node.Type == NodeType.File)
            {
                return node.Size;
            }
            return 0;
        }
    }
}