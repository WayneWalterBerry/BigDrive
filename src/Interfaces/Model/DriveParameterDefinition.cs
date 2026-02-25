// <copyright file="DriveParameterDefinition.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces.Model
{
    using System.Text.Json.Serialization;

    /// <summary>
    /// Represents a single parameter definition that a provider requires
    /// when mounting a new drive. Serialized as part of the JSON array
    /// returned by <see cref="IBigDriveDriveInfo.GetDriveParameters"/>.
    /// </summary>
    public class DriveParameterDefinition
    {
        /// <summary>
        /// Gets or sets the parameter key name. This is used as the key in
        /// <c>DriveConfiguration.Properties</c>.
        /// </summary>
        [JsonPropertyName("name")]
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the user-facing description shown before the input prompt.
        /// </summary>
        [JsonPropertyName("description")]
        public string Description { get; set; }

        /// <summary>
        /// Gets or sets the parameter type. Supported values are "string" (plain text)
        /// and "file" (local file path with Tab completion in the shell).
        /// Defaults to "string" if not specified.
        /// </summary>
        [JsonPropertyName("type")]
        public string Type { get; set; } = "string";
    }
}
