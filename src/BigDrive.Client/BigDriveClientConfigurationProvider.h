// <copyright file="BigDriveClientConfigurationProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <wtypes.h>

class BigDriveClientConfigurationProvider
{
public:
   static HRESULT GetDriveGuids(GUID** ppGuids);
};
