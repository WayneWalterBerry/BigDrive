// <copyright file="Provider.IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Provides file data using the in-memory root structure.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the file data as a stream for the file at the specified path using the root structure.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused in sample).</param>
        /// <param name="path">The file path (e.g., "\A File.txt").</param>
        /// <returns>
        /// An <see cref="IStream"/> containing the file data, or <c>null</c> if not found.
        /// </returns>
        public IStream GetFileData(Guid driveGuid, string path)
        {
            var node = FindNodeByPath(path);
            if (node != null && node.Type == NodeType.File && node.Data != null)
            {
                // Wrap the file's byte[] data in a COM IStream
                return new DataStreamWrapper(new MemoryStream(node.Data));
            }
            return null;
        }
    }
}