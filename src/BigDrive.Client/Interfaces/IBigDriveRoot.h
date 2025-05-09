// <copyright file="IBigDriveRoot.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <Unknwn.h> // For IUnknown
#include <string>
#include <vector>
#include <guiddef.h> // For defining GUIDs

/// <summary>
/// The IID for the IBigDriveRoot interface.
/// </summary>
const IID IID_IBigDriveRoot = { 0xD4E8F3B2, 0x3C4A, 0x4F6A, { 0x9F, 0x3B, 0x2D, 0x4E, 0x8F, 0x3B, 0x2C, 0x4A } };

/// <summary>
/// Represents the interface for retrieving all folders in the root of a drive.
/// </summary>
class __declspec(uuid("D4E8F3B2-3C4A-4F6A-9F3B-2D4E8F3B2C4A")) IBigDriveRoot : public IUnknown
{
public:
    /// <summary>
    /// Retrieves all the folders in the root of the drive.
    /// </summary>
    /// <param name="guid">The registered Drive Identifier.</param>
    /// <param name="folders">A vector to store the folder names.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    virtual HRESULT STDMETHODCALLTYPE GetRootFolders(
        /* [in] */ REFGUID guid,
        /* [out] */ std::vector<std::wstring>& folders) = 0;
};



