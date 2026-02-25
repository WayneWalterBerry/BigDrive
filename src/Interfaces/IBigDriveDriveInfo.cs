// <copyright file="IBigDriveDriveInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for querying drive parameter requirements from a BigDrive provider.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Providers implement this interface to declare the custom parameters they require
    /// when mounting a new drive. The BigDrive Shell queries this interface before creating
    /// a drive so it can prompt the user for the required values.
    /// </para>
    /// <para>
    /// The returned JSON is an array of objects, each with "name", "description", and "type" fields:
    /// <code>
    /// [
    ///   { "name": "ZipFilePath", "description": "Full path to the ZIP file to mount as a drive.", "type": "file" }
    /// ]
    /// </code>
    /// </para>
    /// <para>
    /// Supported type values:
    /// <list type="bullet">
    ///   <item><c>string</c> — A plain text value. The shell prompts with standard input.</item>
    ///   <item><c>file</c> — A local file path. The shell enables Tab file-path completion.</item>
    /// </list>
    /// If "type" is omitted, "string" is assumed.
    /// </para>
    /// <para>
    /// The collected values are sent back as the <c>Properties</c> dictionary in the
    /// <c>DriveConfiguration</c> JSON passed to <c>IBigDriveProvision.Mount(string jsonConfiguration)</c>.
    /// </para>
    /// <para>
    /// This interface is optional. Providers that do not require custom parameters
    /// do not need to implement it.
    /// </para>
    /// </remarks>
    [ComVisible(true)]
    [Guid("3A2B1C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveDriveInfo
    {
        /// <summary>
        /// Gets the parameter definitions required by this provider when mounting a new drive.
        /// </summary>
        /// <returns>
        /// A JSON string containing an array of parameter definitions.
        /// Each element is an object with "name" (the parameter key used in
        /// <c>DriveConfiguration.Properties</c>), "description" (a user-facing
        /// description of the parameter), and "type" (either "string" or "file").
        /// If "type" is omitted, "string" is assumed.
        /// Returns an empty array ("[]") if no custom parameters are required.
        /// </returns>
        string GetDriveParameters();
    }
}
