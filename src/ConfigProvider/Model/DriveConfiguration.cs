// <copyright file="DriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider.Model
{
    using System;
    using System.Collections.Generic;
    using System.Text.Json.Serialization;

    /// <summary>
    /// Represents the configuration for a drive, including its unique identifier, name, CLSID,
    /// and provider-specific properties.
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

        /// <summary>
        /// Gets or sets provider-specific properties for this drive.
        /// These are stored as additional registry values under the drive's key.
        /// </summary>
        /// <remarks>
        /// Reserved property names (id, name, clsid) should not be used as keys.
        /// Properties are stored as REG_SZ values in the registry.
        /// </remarks>
        [JsonIgnore]
        public Dictionary<string, string> Properties { get; set; } = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
    }
}
