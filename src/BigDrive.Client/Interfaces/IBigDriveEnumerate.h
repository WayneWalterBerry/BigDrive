// <copyright file="IBigDriveEnumerate.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <Unknwn.h> // For IUnknown
#include <string>
#include <vector>
#include <guiddef.h> // For defining GUIDs

/// <summary>
/// The IID for the IBigDriveEnumerate interface.
/// </summary>
const IID IID_IBigDriveEnumerate = { 0x457ED786, 0x889A, 0x4C16, { 0xA6, 0xE5, 0x6A, 0x25, 0x01, 0x3D, 0x0A, 0xFA } };

/// <summary>
/// Represents the interface for retrieving all folders in the root of a drive.
/// </summary>
class __declspec(uuid("457ED786-889A-4C16-A6E5-6A25013D0AFA")) IBigDriveEnumerate : public IUnknown
{
public:

    /// <summary>
    /// Retrieves all the folders of the path.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="path">The path to enumerate.</param>
    /// <param name="folders">A vector to store the folder names.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    virtual HRESULT STDMETHODCALLTYPE EnumerateFolders(
        /* [in] */ REFGUID driveGuid,
        /* [in] */ LPWSTR path,
        /* [out] */ SAFEARRAY** folders) = 0;

    /// <summary>
    /// Retrieves all the files of the path.
    /// </summary>
    /// <param name="driveGuid">The registered Drive Identifier.</param>
    /// <param name="path">The path to enumerate.</param>
    /// <param name="files">A vector to store the folder names.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    virtual HRESULT STDMETHODCALLTYPE EnumerateFiles(
        /* [in] */ REFGUID driveGuid,
        /* [in] */ LPWSTR path,
        /* [out] */ SAFEARRAY** files) = 0;
};



