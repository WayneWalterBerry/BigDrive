// <copyright file="IBigDriveFileData.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <Unknwn.h> // For IUnknown
#include <objidl.h> // For IStream
#include <guiddef.h> // For DEFINE_GUID

/// <summary>
/// The IID for the IBigDriveFileData interface.
/// </summary>
const IID IID_IBigDriveFileData = { 0x0F471AE9, 0x1787, 0x437F, { 0xB2, 0x30, 0x60, 0xCA, 0x67, 0x17, 0xDD, 0x04 } };

/// <summary>
/// Represents the interface for retrieving file data as a COM stream.
/// </summary>
class __declspec(uuid("0F471AE9-1787-437F-B230-60CA6717DD04")) IBigDriveFileData : public IUnknown
{
public:

    /// <summary>
    /// Retrieves the file data as an IStream.
    /// </summary>
    /// <param name="ppStream">Address of IStream* to receive the file data stream.</param>
    /// <returns>HRESULT indicating success or failure.</returns>
    virtual HRESULT STDMETHODCALLTYPE GetFileData(
        /* [in] */ REFGUID driveGuid,
        /* [in] */ BSTR path,
        /* [out] */ IStream** ppStream) = 0;
};
