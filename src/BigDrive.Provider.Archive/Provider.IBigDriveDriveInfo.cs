// <copyright file="Provider.IBigDriveDriveInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Archive
{
    using System.Text.Json;

    using BigDrive.Interfaces;
    using BigDrive.Interfaces.Model;

    /// <summary>
    /// Implementation of <see cref="IBigDriveDriveInfo"/> for the Archive provider.
    /// Declares the custom parameters required when mounting an Archive drive.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the parameter definitions required by the Archive provider when mounting a new drive.
        /// </summary>
        /// <returns>
        /// A JSON string containing an array with a single parameter definition
        /// for the ArchiveFilePath property. Supports creating new archives or mounting existing ones.
        /// </returns>
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "ArchiveFilePath",
                    Description = "Full path to the archive file (ZIP, TAR, TAR.GZ, 7z, RAR).",
                    Type = "existing-file"
                }
            };

            return JsonSerializer.Serialize(parameters);
        }
    }
}
