// <copyright file="IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for retrieving file information (metadata).
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Path Format:</strong> The <c>path</c> parameter uses these conventions:
    /// <list type="bullet">
    ///   <item>Paths use backslash (\) as the separator</item>
    ///   <item>Paths are absolute within the drive and start with backslash (e.g., "\folder\file.txt")</item>
    ///   <item>Providers should use <c>path.Trim('\\').Split(...)</c> to parse path segments</item>
    /// </list>
    /// See the Interfaces README.txt PATH FORMAT CONVENTIONS section for implementation guidance.
    /// </para>
    /// </remarks>
    [ComVisible(true)]
    [Guid("A98A0D26-4D5D-4B50-B6FF-8BCB360CB066")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveFileInfo
    {
        /// <summary>
        /// Gets the last modified time of a file.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="path">
        /// Full path to the file within the drive. Uses backslash separator and starts with "\" 
        /// (e.g., "\FolderName\File.txt" or "\File.txt" for files at root).
        /// </param>
        /// <returns>The last modified date and time.</returns>
        DateTime LastModifiedTime(Guid driveGuid, string path);

        /// <summary>
        /// Gets the file size in bytes.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="path">
        /// Full path to the file within the drive. Uses backslash separator and starts with "\" 
        /// (e.g., "\FolderName\File.txt" or "\File.txt" for files at root).
        /// </param>
        /// <returns>The file size in bytes.</returns>
        ulong GetFileSize(Guid driveGuid, string path);
    }
}