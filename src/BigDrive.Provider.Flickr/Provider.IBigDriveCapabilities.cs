// <copyright file="Provider.IBigDriveCapabilities.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using System;

    using BigDrive.Interfaces.Model;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveCapabilities"/> for the Flickr provider.
    /// Advertises which file metadata the Flickr API can supply.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the file metadata capabilities supported by the Flickr provider.
        /// Flickr supports last-modified dates (via DateTaken/DateUploaded/LastUpdated)
        /// but does not expose file sizes through its API.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused; capabilities are the same for all Flickr drives).</param>
        /// <returns><see cref="FileInfoCapabilities.LastModified"/> only.</returns>
        public int GetFileInfoCapabilities(Guid driveGuid)
        {
            return (int)FileInfoCapabilities.LastModified;
        }
    }
}
