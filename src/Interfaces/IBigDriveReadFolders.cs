// <copyright file="IArmClientManager.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System.Runtime.InteropServices;

    [Guid("1894C0CC-A871-40A7-B25E-9A964FC1AF78")]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [ComVisible(true)]
    public interface IBigDriveReadFolders
    {
        /// <summary>
        /// Gets the list of folders.
        /// </summary>
        /// <returns>Array of folder names.</returns>
        string[] GetFolders();
    }
}
