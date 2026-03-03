// <copyright file="DriveParameterSerializer.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces.Serialization
{
    using System;
    using System.Text;

    using BigDrive.Interfaces.Model;

    /// <summary>
    /// Zero-dependency JSON serializer for <see cref="DriveParameterDefinition"/> arrays.
    /// Uses only <see cref="StringBuilder"/> to avoid adding external package references
    /// to the BigDrive.Interfaces assembly, which is loaded early during COM+ activation
    /// before provider assembly resolvers are registered.
    /// </summary>
    public static class DriveParameterSerializer
    {
        /// <summary>
        /// Serializes an array of <see cref="DriveParameterDefinition"/> to a JSON string.
        /// </summary>
        /// <param name="parameters">The parameter definitions to serialize.</param>
        /// <returns>
        /// A JSON array string, e.g.,
        /// <c>[{"name":"Key","description":"Enter key","type":"string"}]</c>.
        /// Returns <c>"[]"</c> if the array is null or empty.
        /// </returns>
        public static string Serialize(DriveParameterDefinition[] parameters)
        {
            if (parameters == null || parameters.Length == 0)
            {
                return "[]";
            }

            StringBuilder sb = new StringBuilder();
            sb.Append('[');

            for (int i = 0; i < parameters.Length; i++)
            {
                if (i > 0)
                {
                    sb.Append(',');
                }

                SerializeParameter(sb, parameters[i]);
            }

            sb.Append(']');
            return sb.ToString();
        }

        /// <summary>
        /// Serializes a single <see cref="DriveParameterDefinition"/> as a JSON object.
        /// </summary>
        /// <param name="sb">The string builder to append to.</param>
        /// <param name="parameter">The parameter definition to serialize.</param>
        private static void SerializeParameter(StringBuilder sb, DriveParameterDefinition parameter)
        {
            sb.Append('{');

            sb.Append("\"name\":");
            AppendJsonString(sb, parameter.Name);

            sb.Append(",\"description\":");
            AppendJsonString(sb, parameter.Description);

            sb.Append(",\"type\":");
            AppendJsonString(sb, DriveParameterTypeToString(parameter.Type));

            sb.Append('}');
        }

        /// <summary>
        /// Appends a JSON-escaped string value (with surrounding quotes) to the builder.
        /// </summary>
        /// <param name="sb">The string builder to append to.</param>
        /// <param name="value">The string value to escape and append.</param>
        private static void AppendJsonString(StringBuilder sb, string value)
        {
            if (value == null)
            {
                sb.Append("null");
                return;
            }

            sb.Append('"');

            for (int i = 0; i < value.Length; i++)
            {
                char c = value[i];
                switch (c)
                {
                    case '"':
                        sb.Append("\\\"");
                        break;
                    case '\\':
                        sb.Append("\\\\");
                        break;
                    case '\n':
                        sb.Append("\\n");
                        break;
                    case '\r':
                        sb.Append("\\r");
                        break;
                    case '\t':
                        sb.Append("\\t");
                        break;
                    default:
                        sb.Append(c);
                        break;
                }
            }

            sb.Append('"');
        }

        /// <summary>
        /// Converts a <see cref="DriveParameterType"/> enum value to its JSON string representation.
        /// </summary>
        /// <param name="type">The parameter type to convert.</param>
        /// <returns>The lowercase/kebab-case string representation.</returns>
        private static string DriveParameterTypeToString(DriveParameterType type)
        {
            switch (type)
            {
                case DriveParameterType.ExistingFile:
                    return "existing-file";
                case DriveParameterType.FilePath:
                    return "filepath";
                case DriveParameterType.Secret:
                    return "secret";
                default:
                    return "string";
            }
        }
    }
}
