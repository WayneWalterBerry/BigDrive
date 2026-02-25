// <copyright file="IBigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for retrieving drive configuration from the BigDrive.Service.
    /// </summary>
    [ComVisible(true)]
    [Guid("D3F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E6F")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveConfiguration
    {
        /// <summary>
        /// Return the configuration for the drive.
        /// </summary>
        /// <param name="guid">Registered Drive Identifier</param>
        /// <returns>JSON string representing the drive configuration.</returns>
        string GetConfiguration(Guid guid);
    }
}
