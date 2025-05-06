// <copyright file="BigDriveReadFolders.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ComObjects
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;
    using System.Threading;
    using BigDrive.Interfaces;

    [Guid("D4A1C5E3-3B6A-4F2A-8A1E-5B6C3D9F7E2A")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public class BigDriveReadFolders : ServicedComponent, IBigDriveReadFolders
    {
        public string[] GetFolders(Guid guid, string path, CancellationToken cancellationToken)
        {
            // Example implementation
            return new string[] { "Folder1", "Folder2", "Folder3" };
        }
    }
}
