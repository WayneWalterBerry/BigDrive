// <copyright file="DriveParameterDefinition.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces.Model
{
    /// <summary>
    /// Represents a single parameter definition that a provider requires
    /// when mounting a new drive. Serialized as part of the JSON array
    /// returned by <see cref="IBigDriveDriveInfo.GetDriveParameters"/>.
    /// </summary>
    /// <remarks>
    /// Use <see cref="Serialization.DriveParameterSerializer.Serialize"/> to
    /// convert an array of definitions to JSON.
    /// </remarks>
    public class DriveParameterDefinition
    {
        /// <summary>
        /// Gets or sets the parameter key name. This is used as the key in
        /// <c>DriveConfiguration.Properties</c>.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the user-facing description shown before the input prompt.
        /// </summary>
        public string Description { get; set; }

        /// <summary>
        /// Gets or sets the parameter type. Controls how the shell prompts for
        /// and validates user input.
        /// </summary>
        /// <seealso cref="DriveParameterType"/>
        public DriveParameterType Type { get; set; } = DriveParameterType.String;
    }
}
