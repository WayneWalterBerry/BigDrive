// <copyright file="BigDriveConfigurationClient.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once  

#include <string>  
#include <wtypes.h>

class BigDriveConfigurationClient
{
public:  

    static HRESULT GetConfiguration(GUID guid, LPWSTR* pszConfiguration);

};