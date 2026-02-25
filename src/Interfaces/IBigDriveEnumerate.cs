// <copyright file="IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for enumerating folders and files within a BigDrive virtual drive.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Path Format:</strong> The <paramref name="path"/> parameter uses these conventions:
    /// <list type="bullet">
    ///   <item>Paths use backslash (\) as the separator</item>
    ///   <item>Paths are absolute within the drive and start with backslash (e.g., "\folder")</item>
    ///   <item>Root is represented as "\" or "\\"</item>
    ///   <item>Providers should use <c>path.Trim('\\').Split(...)</c> to parse path segments</item>
    /// </list>
    /// </para>
    /// <para>
    /// <strong>Return Values:</strong> Methods return item NAMES only (e.g., "MyFolder", "File.txt"),
    /// NOT full paths. The shell constructs full paths by combining the enumeration path with returned names.
    /// </para>
    /// <para>
    /// See the Interfaces README.txt PATH FORMAT CONVENTIONS section for implementation guidance.
    /// </para>
    /// </remarks>
    [ComVisible(true)]
    [Guid("457ED786-889A-4C16-A6E5-6A25013D0AFA")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveEnumerate
    {
        /// <summary>
        /// Return all the folders in the specified path.
        /// </summary>
        /// <param name="driveGuid">Registered Drive Identifier.</param>
        /// <param name="path">
        /// Path to enumerate. Uses backslash separator, starts with "\" (e.g., "\", "\FolderName").
        /// </param>
        /// <returns>
        /// Array of folder names (not full paths) at the specified path.
        /// Example: For path "\Parent", returns ["Child1", "Child2"], not ["\Parent\Child1", ...].
        /// </returns>
        string[] EnumerateFolders(Guid driveGuid, string path);

        /// <summary>
        /// Returns all the files in the specified path.
        /// </summary>
        /// <param name="driveGuid">Registered Drive Identifier.</param>
        /// <param name="path">
        /// Path to enumerate. Uses backslash separator, starts with "\" (e.g., "\", "\FolderName").
        /// </param>
        /// <returns>
        /// Array of file names (not full paths) at the specified path.
        /// Example: For path "\", returns ["File1.txt", "File2.txt"], not ["\File1.txt", ...].
        /// </returns>
        string[] EnumerateFiles(Guid driveGuid, string path);
    }
}
