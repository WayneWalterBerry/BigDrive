// <copyright file="IBigDriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;
    using System.Threading;

    [ComVisible(true)]
    [Guid("D3F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E6F")] // Randomly generated GUID
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveConfiguration
    {
        /// <summary>
        /// Return the configuration for the drive.
        /// </summary>
        /// <param name="guid">Registered Drive Identifier</param>
        /// <returns></returns>
        string GetConfiguration(Guid guid);
    }
}
