// <copyright file="IBigDriveEnumerate.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    [ComVisible(true)]
    [Guid("457ED786-889A-4C16-A6E5-6A25013D0AFA")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveEnumerate
    {
        /// <summary>
        /// Return all the folders in the root.
        /// </summary>
        /// <param name="driveGuid">Registered Drive Identifier</param>
        /// <param name="path">Path to Enumerate</param>
        /// <returns>Folder Names</returns>
        string[] EnumerateFolders(Guid driveGuid, string path);
    }
}
