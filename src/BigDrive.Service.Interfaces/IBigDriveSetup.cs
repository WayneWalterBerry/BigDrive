// <copyright file="IBigDriveSetup.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Service.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for setup validation of the BigDrive.Service.
    /// </summary>
    [ComVisible(true)]
    [Guid("54B5E354-7982-4AC7-8D82-37C27E190113")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveSetup
    {
        /// <summary>
        /// Called By BigDrive.Setup to validate the setup process.
        /// </summary>
        /// <param name="activityId">The activity identifier for tracing.</param>
        void Validate(Guid activityId);
    }
}
