﻿// <copyright file="IBigDriveRoot.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    [ComVisible(true)]
    [Guid("D4E8F3B2-3C4A-4F6A-9F3B-2D4E8F3B2C4A")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveRoot
    {
        /// <summary>
        /// Return all the folders in the root.
        /// </summary>
        /// <param name="guid">Registered Drive Identifier</param>
        /// <returns>Folder Names</returns>
        string[] GetRootFolders(Guid guid);
    }
}
