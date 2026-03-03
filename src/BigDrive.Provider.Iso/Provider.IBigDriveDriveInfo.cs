// <copyright file="Provider.IBigDriveDriveInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Iso
{
    using BigDrive.Interfaces;
    using BigDrive.Interfaces.Model;
    using BigDrive.Interfaces.Serialization;

    /// <summary>
    /// Implementation of <see cref="IBigDriveDriveInfo"/> for the ISO provider.
    /// Declares the custom parameters required when mounting an ISO drive.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the parameter definitions required by the ISO provider when mounting a new drive.
        /// </summary>
        /// <returns>
        /// A JSON string containing an array with a single parameter definition
        /// for the IsoFilePath property.
        /// </returns>
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "IsoFilePath",
                    Description = "Full path to the ISO file (ISO 9660, Joliet, UDF).",
                    Type = DriveParameterType.ExistingFile
                }
            };

            return DriveParameterSerializer.Serialize(parameters);
        }
    }
}
