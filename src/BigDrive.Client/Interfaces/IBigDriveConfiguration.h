// <copyright file="IBigDriveConfiguration.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include <iostream>
#include <windows.h>
#include <comdef.h>
#include <objbase.h>

// {E6F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E7F}
const CLSID CLSID_BigDriveConfiguration = { 0xE6F5A1B2, 0x4C6E, 0x4F8A, { 0x9D, 0x3E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x7F } };

// {D3F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E6F}
const IID IID_IBigDriveConfiguration = { 0xD3F5A1B2, 0x4C6E, 0x4F8A, { 0x9D, 0x3E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F } };

// Define the IBigDriveConfiguration interface
interface IBigDriveConfiguration : public IUnknown
{
    virtual HRESULT __stdcall GetConfiguration(
        /* [in] */ GUID guid,
        /* [out, retval] */ BSTR * configuration) = 0;
};

