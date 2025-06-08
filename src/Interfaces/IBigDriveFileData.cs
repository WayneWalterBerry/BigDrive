// <copyright file="IBigDriveFileData.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace BigDrive.Interfaces
{
    /// <summary>
    /// Interface for retrieving file data as a COM stream.
    /// </summary>
    [ComVisible(true)]
    [Guid("0F471AE9-1787-437F-B230-60CA6717DD04")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveFileData
    {
        /// <summary>
        /// Retrieves the file data as an IStream.
        /// </summary>
        /// <param name="driveGuid">Registered Drive Identifier</param>
        /// <param name="path">Path to Enumerate</param> 
        /// <param name="stream">When this method returns, contains the IStream with the file data.</param>
        /// <returns>HRESULT indicating success or failure.</returns>
        int GetBlob(Guid driveGuid, string path, out IStream stream);
    }
}