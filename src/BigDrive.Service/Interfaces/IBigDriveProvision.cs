// <copyright file="IBigDriveProvision.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    [ComVisible(true)]
    [Guid("293D4995-FDFB-46FD-A0C6-A7DE2DA5B13F")] // Randomly generated GUID
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveProvision
    {
        /// <summary>
        /// Create a new drive with the specifed guid.
        /// </summary>
        void Create(Guid driveGuid);
    }
}
