// <copyright file="DriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider.Model
{
    using System;
    using System.Text.Json.Serialization;

    /// <summary>
    /// Represents the configuration for a drive, including its unique identifier, name, and CLSID.
    /// </summary>
    public class DriveConfiguration
    {
        /// <summary>
        /// Gets or sets the unique identifier for the drive.
        /// </summary>
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        /// <summary>
        /// Gets or sets the name of the drive.
        /// </summary>
        [JsonPropertyName("name")]
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the CLSID (Class Identifier) associated with the drive.
        /// </summary>
        [JsonPropertyName("clsid")]
        public Guid CLSID { get; set; }
    }
}
