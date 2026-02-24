// <copyright file="ProviderFactory.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Runtime.InteropServices;

    using BigDrive.ConfigProvider;
    using BigDrive.ConfigProvider.Model;
    using BigDrive.Interfaces;

    /// <summary>
    /// Factory for creating provider instances from drive configurations.
    /// </summary>
    public static class ProviderFactory
    {
        /// <summary>
        /// Creates an IBigDriveEnumerate instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveEnumerate interface, or null if not available.</returns>
        public static IBigDriveEnumerate GetEnumerateProvider(Guid driveGuid)
        {
            object provider = GetProviderInstance(driveGuid);
            return provider as IBigDriveEnumerate;
        }

        /// <summary>
        /// Creates an IBigDriveFileOperations instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveFileOperations interface, or null if not available.</returns>
        public static IBigDriveFileOperations GetFileOperationsProvider(Guid driveGuid)
        {
            object provider = GetProviderInstance(driveGuid);
            return provider as IBigDriveFileOperations;
        }

        /// <summary>
        /// Creates an IBigDriveFileData instance for the specified drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The IBigDriveFileData interface, or null if not available.</returns>
        public static IBigDriveFileData GetFileDataProvider(Guid driveGuid)
        {
            object provider = GetProviderInstance(driveGuid);
            return provider as IBigDriveFileData;
        }

        /// <summary>
        /// Gets the raw provider instance for a drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>The provider COM object.</returns>
        private static object GetProviderInstance(Guid driveGuid)
        {
            DriveConfiguration config = DriveManager.ReadConfiguration(driveGuid, System.Threading.CancellationToken.None);
            if (config == null)
            {
                throw new InvalidOperationException("Drive configuration not found for GUID: " + driveGuid);
            }

            Type providerType = Type.GetTypeFromCLSID(config.CLSID);
            if (providerType == null)
            {
                throw new InvalidOperationException("Provider type not found for CLSID: " + config.CLSID);
            }

            object provider = Activator.CreateInstance(providerType);
            if (provider == null)
            {
                throw new InvalidOperationException("Failed to create provider instance for CLSID: " + config.CLSID);
            }

            return provider;
        }
    }
}
