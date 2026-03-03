// <copyright file="Provider.IBigDriveCapabilities.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using System;

    using BigDrive.Interfaces.Model;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveCapabilities"/> for the ISO provider.
    /// Advertises which file metadata the ISO provider can supply.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the file metadata capabilities supported by the ISO provider.
        /// ISO images contain both file sizes and last-modified timestamps.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused; capabilities are the same for all ISO drives).</param>
        /// <returns><see cref="FileInfoCapabilities.All"/>.</returns>
        public int GetFileInfoCapabilities(Guid driveGuid)
        {
            return (int)FileInfoCapabilities.All;
        }
    }
}
