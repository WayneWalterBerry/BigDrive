// <copyright file="FileStoreFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell.FileStores
{
    using System;

    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Creates IFileStore instances based on PathInfo.
    /// Resolves whether a path targets the local filesystem or a BigDrive provider.
    /// </summary>
    public static class FileStoreFactory
    {
        /// <summary>
        /// Creates the appropriate IFileStore for the given path info.
        /// </summary>
        /// <param name="pathInfo">The parsed path information.</param>
        /// <param name="context">The shell context.</param>
        /// <returns>The IFileStore instance, or null if the path cannot be resolved.</returns>
        public static IFileStore Create(PathInfo pathInfo, ShellContext context)
        {
            if (pathInfo.IsOSDrive)
            {
                return new LocalFileStore(pathInfo.DriveLetter);
            }

            if (pathInfo.IsBigDrive)
            {
                DriveConfiguration config = context.DriveLetterManager.GetDriveConfiguration(pathInfo.DriveLetter);
                if (config == null)
                {
                    return null;
                }

                return new BigDriveFileStore(config);
            }

            return null;
        }
    }
}
