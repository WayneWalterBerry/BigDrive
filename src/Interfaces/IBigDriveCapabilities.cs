// <copyright file="IBigDriveCapabilities.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Interfaces
{
    using System;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for querying a provider's metadata capabilities at runtime.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Providers implement this interface to advertise which file metadata they can
    /// supply (file sizes, last-modified dates, etc.). The BigDrive Shell queries this
    /// interface after a drive is mounted and uses the result to adapt its display —
    /// for example, hiding the Length column when the provider cannot return file sizes.
    /// </para>
    /// <para>
    /// This interface is optional. Providers that do not implement it are assumed to
    /// support all capabilities (<c>FileInfoCapabilities.All</c>).
    /// </para>
    /// <para>
    /// <strong>COM Marshaling Note:</strong> The return type is <c>int</c> rather than
    /// the <c>FileInfoCapabilities</c> enum because enum types do not marshal reliably
    /// across out-of-process COM IUnknown boundaries. The Shell casts the returned
    /// <c>int</c> to <see cref="Model.FileInfoCapabilities"/> on the consumer side.
    /// </para>
    /// </remarks>
    [ComVisible(true)]
    [Guid("D4E5F6A7-B8C9-4D0E-A1F2-3B4C5D6E7F80")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IBigDriveCapabilities
    {
        /// <summary>
        /// Gets the file metadata capabilities supported by this provider for the specified drive.
        /// </summary>
        /// <param name="driveGuid">
        /// The drive GUID to query capabilities for. Providers may return different
        /// capabilities per drive (e.g., a free-tier account may not expose file sizes).
        /// Pass <see cref="Guid.Empty"/> to query provider-level defaults.
        /// </param>
        /// <returns>
        /// An <c>int</c> containing <see cref="Model.FileInfoCapabilities"/> flags indicating which
        /// <see cref="IBigDriveFileInfo"/> methods return meaningful data.
        /// The Shell casts this to <see cref="Model.FileInfoCapabilities"/> and uses it to hide
        /// columns that the provider cannot populate (e.g., suppress the Length column when
        /// <see cref="Model.FileInfoCapabilities.FileSize"/> is not set).
        /// </returns>
        int GetFileInfoCapabilities(Guid driveGuid);
    }
}
