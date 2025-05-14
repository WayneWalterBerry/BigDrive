// <copyright file="IBigDriveRegistration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    [ComVisible(true)]
    [Guid("FF0FA03A-5DC1-464F-AFCE-5C60ECAA3912")] // Randomly generated GUID
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveRegistration
    {
        /// <summary>
        /// Called By Installation To Tell The COM+ Application To Register
        /// </summary>
        void Register();
    }
}
