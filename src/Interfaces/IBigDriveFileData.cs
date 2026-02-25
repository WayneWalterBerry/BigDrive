// <copyright file="IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Interface for retrieving file data as a COM stream.
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
    [Guid("0F471AE9-1787-437F-B230-60CA6717DD04")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveFileData
    {
        /// <summary>
        /// Retrieves the file data as an IStream.
        /// </summary>
        /// <param name="driveGuid">Registered Drive Identifier.</param>
        /// <param name="path">
        /// Full path to the file within the drive. Uses backslash separator and starts with "\" 
        /// (e.g., "\FolderName\File.txt" or "\File.txt" for files at root).
        /// </param> 
        /// <param name="stream">When this method returns, contains the IStream with the file data.</param>
        /// <returns>HRESULT indicating success or failure.</returns>
        [PreserveSig]
        int GetFileData(Guid driveGuid, string path, [MarshalAs(UnmanagedType.Interface)] out IStream stream);
    }
}