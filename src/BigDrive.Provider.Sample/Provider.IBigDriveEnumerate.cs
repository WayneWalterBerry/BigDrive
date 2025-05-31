// <copyright file="Provider.BigDriveRoot.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Sample
{
    using System;

    /// <summary>
    /// Empty implementation of <see cref="IBigDriveEnumerate"/> for the sample provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc />
        public string[] EnumerateFolders(Guid driveGuid, string path)
        {
            return new string[] { "RootFolder1", "RootFolder2", "RootFolder3" };
        }

        /// <inheritdoc />
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            return new string[] { "Root File 1.txt", "Root File 2.txt", "Root File 3.txt" };
        }
    }
}
