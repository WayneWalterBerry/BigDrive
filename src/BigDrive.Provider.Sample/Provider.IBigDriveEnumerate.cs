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
            if (path == "\\")
            {
                return new string[] { "RootFolder1", "RootFolder2", "RootFolder3" };
            }
            else if (path == "\\RootFolder1")
            {
                return new string[] { "SubFolder1" };
            }
            else if (path == "\\RootFolder2")
            {
                return new string[] { "SubFolder2" };
            }
            else if (path == "\\RootFolder2\\Folder2")
            {
                return new string[] { "Folder2" };
            }

            return Array.Empty<string>();
        }

        /// <inheritdoc />
        public string[] EnumerateFiles(Guid driveGuid, string path)
        {
            if (path == "\\")
            {
                return new string[] { "A File.txt", "Root File 2.txt", "Z File.txt" };
            }

            return Array.Empty<string>();
        }
    }
}
