// <copyright file="Provider.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using System;
    using System.EnterpriseServices;
    using System.Runtime.InteropServices;

    using BigDrive.Interfaces;

    /// <summary>
    /// BigDrive provider for virtual disk image files (VHD, VHDX, VMDK, VDI).
    /// </summary>
    /// <remarks>
    /// This provider enables browsing and modifying virtual machine disk images without
    /// mounting them in Hyper-V, VMware, or VirtualBox. Supports multiple disk formats
    /// and file systems (NTFS, FAT32, exFAT, ext2/3/4).
    ///
    /// Drive-specific properties (stored in registry under each drive):
    /// - VhdFilePath: Full path to the VHD/VHDX/VMDK/VDI file
    /// - PartitionIndex: Zero-based partition index to mount (default: 0)
    /// - ReadOnly: Mount in read-only mode (default: false)
    /// </remarks>
    [Guid("D1E2F3A4-B5C6-7D8E-9F0A-1B2C3D4E5F67")]
    [ClassInterface(ClassInterfaceType.None)]
    [ComVisible(true)]
    public partial class Provider : ServicedComponent,
        IProcessInitializer,
        IBigDriveRegistration,
        IBigDriveDriveInfo,
        IBigDriveCapabilities,
        IBigDriveEnumerate,
        IBigDriveFileInfo,
        IBigDriveFileData,
        IBigDriveFileOperations
    {
        /// <summary>
        /// The trace source for logging.
        /// </summary>
        private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

        /// <summary>
        /// Static constructor to ensure AssemblyResolver is initialized early.
        /// </summary>
        static Provider()
        {
            AssemblyResolver.Initialize();
        }

        /// <summary>
        /// Gets the CLSID of this provider.
        /// </summary>
        public static Guid CLSID
        {
            get
            {
                Type providerType = typeof(Provider);
                GuidAttribute guidAttribute = (GuidAttribute)Attribute.GetCustomAttribute(providerType, typeof(GuidAttribute));
                return Guid.Parse(guidAttribute.Value);
            }
        }

        /// <summary>
        /// Gets the provider configuration for registry registration.
        /// </summary>
        private static BigDrive.ConfigProvider.Model.ProviderConfiguration ProviderConfig
        {
            get
            {
                return ProviderConfigurationFactory.Create();
            }
        }

        /// <summary>
        /// Gets the client wrapper for a specific drive.
        /// </summary>
        /// <param name="driveGuid">The drive GUID.</param>
        /// <returns>A client wrapper configured for the drive.</returns>
        private static VirtualDiskClientWrapper GetClient(Guid driveGuid)
        {
            return VirtualDiskClientWrapper.GetForDrive(driveGuid);
        }

        /// <summary>
        /// Normalizes a path by trimming leading/trailing separators and converting to forward slashes.
        /// Returns an empty string for root paths.
        /// </summary>
        /// <param name="path">The path to normalize.</param>
        /// <returns>The normalized path, or empty string for root.</returns>
        private static string NormalizePath(string path)
        {
            if (string.IsNullOrEmpty(path) || path == "\\" || path == "/" || path == "//")
            {
                return string.Empty;
            }

            return path.Trim('\\', '/').Replace('\\', '/');
        }
    }
}
