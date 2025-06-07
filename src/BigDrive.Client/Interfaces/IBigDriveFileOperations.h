// <copyright file="IBigDriveFileOperations.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <objbase.h>
#include <shlobj.h>

// {7BE23F90-8D32-4D88-B4E7-59BFDA941F04}
const IID IID_IBigDriveFileOperations =
    { 0x7be23f90, 0x8d32, 0x4d88, 0xb4, 0xe7, 0x59, 0xbf, 0xda, 0x94, 0x1f, 0x4 };

/// <summary>
/// Interface for performing file operations on BigDrive files and folders.
/// </summary>
class IBigDriveFileOperations : public IUnknown
{
public:
    /// <summary>
    /// Copies a local file to the BigDrive storage.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="localFilePath">The local file path to copy from.</param>
    /// <param name="bigDriveTargetPath">The destination path in BigDrive.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    virtual HRESULT STDMETHODCALLTYPE CopyFileToBigDrive(
        REFCLSID driveGuid,
        LPCWSTR localFilePath,
        LPCWSTR bigDriveTargetPath) = 0;

    /// <summary>
    /// Copies a BigDrive file to a local storage.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="bigDriveFilePath">The BigDrive file path to copy from.</param>
    /// <param name="localTargetPath">The destination path in local storage.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    virtual HRESULT STDMETHODCALLTYPE CopyFileFromBigDrive(
        REFCLSID driveGuid,
        LPCWSTR bigDriveFilePath,
        LPCWSTR localTargetPath) = 0;

    /// <summary>
    /// Deletes a file or folder from BigDrive storage.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="bigDriveFilePath">The file path to delete.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    virtual HRESULT STDMETHODCALLTYPE DeleteFile(
        REFCLSID driveGuid,
        LPCWSTR bigDriveFilePath) = 0;

    /// <summary>
    /// Creates a new directory in BigDrive storage.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="bigDriveDirectoryPath">The directory path to create.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    virtual HRESULT STDMETHODCALLTYPE CreateDirectory(
        REFCLSID driveGuid,
        LPCWSTR bigDriveDirectoryPath) = 0;

    /// <summary>
    /// Opens a BigDrive file with the associated application.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="bigDriveFilePath">The file path to open.</param>
    /// <param name="hwndParent">Parent window handle for any UI.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    virtual HRESULT STDMETHODCALLTYPE OpenFile(
        REFCLSID driveGuid,
        LPCWSTR bigDriveFilePath,
        HWND hwndParent) = 0;

    /// <summary>
    /// Moves a file within the BigDrive storage.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="sourcePath">The source file path.</param>
    /// <param name="destinationPath">The destination file path.</param>
    /// <returns>S_OK if successful; otherwise, an error code.</returns>
    virtual HRESULT STDMETHODCALLTYPE MoveFile(
        REFCLSID driveGuid,
        LPCWSTR sourcePath,
        LPCWSTR destinationPath) = 0;
};

// Add this method to BigDriveInterfaceProvider.h and implement in BigDriveInterfaceProvider.cpp
//