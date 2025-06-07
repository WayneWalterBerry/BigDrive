// <copyright file="IBigDriveRegistration.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <Unknwn.h> // For IUnknown


// IID for IBigDriveRegistration: {FF0FA03A-5DC1-464F-AFCE-5C60ECAA3912}
const IID IID_IBigDriveRegistration =
    { 0xFF0FA03A, 0x5DC1, 0x464F, { 0xAF, 0xCE, 0x5C, 0x60, 0xEC, 0xAA, 0x39, 0x12 } };

// {FF0FA03A-5DC1-464F-AFCE-5C60ECAA3912}
struct __declspec(uuid("FF0FA03A-5DC1-464F-AFCE-5C60ECAA3912"))
    IBigDriveRegistration : public IUnknown
{
    // Called By Installation To Tell The COM+ Application To Register
    virtual HRESULT STDMETHODCALLTYPE Register() = 0;
};