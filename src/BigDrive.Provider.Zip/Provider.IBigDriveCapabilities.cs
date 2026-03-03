// <copyright file="Provider.IBigDriveCapabilities.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System;

    using BigDrive.Interfaces.Model;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveCapabilities"/> for the Zip provider.
    /// Advertises which file metadata the Zip provider can supply.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the file metadata capabilities supported by the Zip provider.
        /// Zip archives contain both file sizes and last-modified timestamps.
        /// </summary>
        /// <param name="driveGuid">The drive GUID (unused; capabilities are the same for all Zip drives).</param>
        /// <returns><see cref="FileInfoCapabilities.All"/>.</returns>
        public int GetFileInfoCapabilities(Guid driveGuid)
        {
            return (int)FileInfoCapabilities.All;
        }
    }
}
