// <copyright file="IBigDriveFileInfo.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <string>
#include <vector>
#include <guiddef.h> // For defining GUIDs
#include "oleauto.h."

/// <summary>
/// The IID for the IBigDriveEnumerate interface.
/// </summary>
const IID IID_IBigDriveFileInfo = { 0xA98A0D26, 0x4D5D, 0x4B50, { 0xB6, 0xFF, 0x8B, 0xCB, 0x36, 0x0C, 0xB0, 0x66 } };

/// <summary>
/// Represents the interface for retrieving all folders in the root of a drive.
/// </summary>
class __declspec(uuid("A98A0D26-4D5D-4B50-B6FF-8BCB360CB066")) IBigDriveFileInfo : public IUnknown
{
public:

    virtual HRESULT STDMETHODCALLTYPE LastModifiedTime(
        /* [in] */ REFGUID driveGuid,
        /* [in] */ LPWSTR path,
        /* [out] */ DATE* pDATE) = 0;
};



