// <copyright file="DriveConfigurationExtensions.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <summary>
// Provides extension methods for the DriveConfiguration class, including JSON serialization.
// </summary>

namespace BigDrive.ConfigProvider.Extensions
{
    using System.Text.Json;
    using System.Text.Json.Serialization;
    using BigDrive.ConfigProvider.Model;

    /// <summary>
    /// Contains extension methods for the <see cref="DriveConfiguration"/> class.
    /// </summary>
    public static class DriveConfigurationExtensions
    {
        /// <summary>
        /// Converts the <see cref="DriveConfiguration"/> object to its JSON string representation.
        /// </summary>
        /// <param name="driveConfiguration">The <see cref="DriveConfiguration"/> object to serialize.</param>
        /// <returns>A JSON string representation of the <see cref="DriveConfiguration"/> object.</returns>
        public static string ToJson(this DriveConfiguration driveConfiguration)
        {
            // Serialize the configuration to JSON
            var options = new JsonSerializerOptions
            {
                WriteIndented = false,
                Converters =
                {
                    new JsonStringEnumConverter(JsonNamingPolicy.CamelCase),
                }
            };

            string json = JsonSerializer.Serialize(driveConfiguration, options);
            json = json.Replace("\r", "").Replace("\n", "");

            return json;
        }
    }
}
