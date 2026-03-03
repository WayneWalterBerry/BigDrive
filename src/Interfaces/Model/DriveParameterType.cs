// <copyright file="DriveParameterType.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces.Model
{
    /// <summary>
    /// Specifies the type of a drive parameter, controlling how the BigDrive Shell
    /// prompts for and validates user input during mount.
    /// </summary>
    /// <remarks>
    /// Providers set this on <see cref="DriveParameterDefinition.Type"/> to control
    /// how the shell collects and stores the value. Use
    /// <see cref="Serialization.DriveParameterSerializer"/> to serialize definitions
    /// to JSON.
    /// </remarks>
    public enum DriveParameterType
    {
        /// <summary>
        /// A plain text value. The shell prompts with standard input.
        /// </summary>
        String,

        /// <summary>
        /// A local file path that must exist. The shell enables Tab completion
        /// and validates existence.
        /// </summary>
        ExistingFile,

        /// <summary>
        /// A local file path that may or may not exist. The shell enables Tab
        /// completion but allows non-existent paths (for creating new files).
        /// </summary>
        FilePath,

        /// <summary>
        /// A secret value such as an API key or password. The shell masks input
        /// with asterisk (*) characters to prevent the value from being displayed.
        /// </summary>
        Secret
    }
}
