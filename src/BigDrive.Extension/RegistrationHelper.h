// <copyright file="RegistrationHelper.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

HRESULT TakeOwnershipAndGrantFullControl(LPCWSTR keyPath, HRESULT(*callback)());
