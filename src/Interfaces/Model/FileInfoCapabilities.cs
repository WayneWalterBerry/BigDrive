// <copyright file="FileInfoCapabilities.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces.Model
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Flags indicating which file metadata a provider can supply.
    /// Returned by <see cref="IBigDriveDriveInfo.GetFileInfoCapabilities"/> so the
    /// Shell can adapt its display (e.g., hide the Length column when
    /// <see cref="FileSize"/> is not set).
    /// </summary>
    /// <remarks>
    /// Providers that do not implement <see cref="IBigDriveDriveInfo"/> are assumed
    /// to support all capabilities (<see cref="All"/>).
    /// </remarks>
    [Flags]
    public enum FileInfoCapabilities
    {
        /// <summary>
        /// The provider cannot supply any file metadata.
        /// </summary>
        None = 0,

        /// <summary>
        /// The provider can return file sizes via
        /// <see cref="IBigDriveFileInfo.GetFileSize"/>.
        /// </summary>
        FileSize = 1,

        /// <summary>
        /// The provider can return last-modified timestamps via
        /// <see cref="IBigDriveFileInfo.LastModifiedTime"/>.
        /// </summary>
        LastModified = 2,

        /// <summary>
        /// The provider supports all file metadata capabilities.
        /// </summary>
        All = FileSize | LastModified
    }
}
