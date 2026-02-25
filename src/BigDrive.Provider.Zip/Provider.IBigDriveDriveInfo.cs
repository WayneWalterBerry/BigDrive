// <copyright file="Provider.IBigDriveDriveInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Zip
{
    using System.Text.Json;

    using BigDrive.Interfaces;
    using BigDrive.Interfaces.Model;

    /// <summary>
    /// Implementation of <see cref="IBigDriveDriveInfo"/> for the Zip provider.
    /// Declares the custom parameters required when mounting a Zip drive.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the parameter definitions required by the Zip provider when mounting a new drive.
        /// </summary>
        /// <returns>
        /// A JSON string containing an array with a single parameter definition
        /// for the ZipFilePath property. Uses "filepath" type to allow creating new ZIP files.
        /// </returns>
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "ZipFilePath",
                    Description = "Full path to the ZIP file (existing or new).",
                    Type = "filepath"
                }
            };

            return JsonSerializer.Serialize(parameters);
        }
    }
}
