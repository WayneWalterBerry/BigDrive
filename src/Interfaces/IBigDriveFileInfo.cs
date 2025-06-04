// <copyright file="IBigDriveFileInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for retrieving file information.
    /// </summary>
    [ComVisible(true)]
    [Guid("A98A0D26-4D5D-4B50-B6FF-8BCB360CB066")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveFileInfo
    {
        /// <summary>
        /// Gets the last modified time of a file.
        /// </summary>
        /// <param name="driveGuid">The registered Drive Identifier.</param>
        /// <param name="path">The path to the file.</param>
        /// <returns>The last modified date and time.</returns>
        DateTime LastModifiedTime(Guid driveGuid, string path);
    }
}