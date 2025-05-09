// <copyright file="DriveConfiguration.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.ConfigProvider
{
    using System;
    using System.Text.Json;
    using System.Text.Json.Serialization;

    /// <summary>
    /// A custom JSON converter for <see cref="Guid"/> that ensures GUIDs are serialized
    /// with brackets (e.g., "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}") and deserialized correctly.
    /// </summary>
    public class GuidWithBracketsConverter : JsonConverter<Guid>
    {
        /// <summary>
        /// Reads and converts the JSON string representation of a GUID into a <see cref="Guid"/> object.
        /// </summary>
        /// <param name="reader">The <see cref="Utf8JsonReader"/> to read the JSON data from.</param>
        /// <param name="typeToConvert">The type of the object to convert (always <see cref="Guid"/>).</param>
        /// <param name="options">Options to control the behavior of the JSON serializer.</param>
        /// <returns>A <see cref="Guid"/> object parsed from the JSON string.</returns>
        public override Guid Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            return Guid.Parse(reader.GetString());
        }

        /// <summary>
        /// Writes a <see cref="Guid"/> object to its JSON string representation with brackets.
        /// </summary>
        /// <param name="writer">The <see cref="Utf8JsonWriter"/> to write the JSON data to.</param>
        /// <param name="value">The <see cref="Guid"/> value to serialize.</param>
        /// <param name="options">Options to control the behavior of the JSON serializer.</param>
        public override void Write(Utf8JsonWriter writer, Guid value, JsonSerializerOptions options)
        {
            writer.WriteStringValue(value.ToString("B")); // "B" format includes brackets
        }
    }
}
