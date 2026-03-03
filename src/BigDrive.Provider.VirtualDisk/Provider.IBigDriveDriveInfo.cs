// <copyright file="Provider.IBigDriveDriveInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.VirtualDisk
{
    using BigDrive.Interfaces.Model;
    using BigDrive.Interfaces.Serialization;

    /// <summary>
    /// Implementation of <see cref="BigDrive.Interfaces.IBigDriveDriveInfo"/> for the VirtualDisk provider.
    /// </summary>
    public partial class Provider
    {
        /// <inheritdoc/>
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "VhdFilePath",
                    Description = "Full path to the virtual disk file (VHD, VHDX, VMDK, or VDI format, e.g., C:\\VMs\\disk.vhdx).",
                    Type = DriveParameterType.ExistingFile
                },
                new DriveParameterDefinition
                {
                    Name = "PartitionIndex",
                    Description = "Zero-based partition index to mount (default: 0 for first partition). Use -1 to list all partitions.",
                    Type = DriveParameterType.String
                },
                new DriveParameterDefinition
                {
                    Name = "ReadOnly",
                    Description = "Mount the disk in read-only mode (true/false, default: false). Recommended for active VM disks.",
                    Type = DriveParameterType.String
                }
            };

            return DriveParameterSerializer.Serialize(parameters);
        }
    }
}
