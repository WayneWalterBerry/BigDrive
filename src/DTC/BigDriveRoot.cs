// <copyright file="BigDriveRoot.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ComObjects
{
    using System;
    using System.Collections.Generic;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;
    using System.Threading;
    using BigDrive.Interfaces;
    using ConfigProvider;

    [Guid("E3A1B5C4-2D6A-4F3A-9B1E-6C3D8F7E2B1A")] // Unique GUID for the COM class
    [ClassInterface(ClassInterfaceType.None)] // No automatic interface generation
    [ComVisible(true)] // Make the class visible to COM
    public class BigDriveRoot : ServicedComponent, IBigDriveRoot
    {
        /// <inheritdoc/>
        public string[] GetFolders(Guid guid, CancellationToken cancellationToken)
        {
            var driveConfiguration = ConfigurationProvider.ReadConfiguration(guid, cancellationToken);

            // Simulate fetching folder names based on the provided GUID.
            // In a real implementation, this would query a database, file system, or other data source.

            // Example logic: Return a static list of folder names for demonstration purposes.
            var folderNames = new List<string>
            {
                "Folder1",
                "Folder2",
                "Folder3"
            };

            // You can add logic here to filter or modify the folder list based on the GUID if needed.

            return folderNames.ToArray();
        }
    }
}
