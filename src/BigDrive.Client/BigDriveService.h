// <copyright file="BigDriveService.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>

EXTERN_C const IID IID_IBigDriveProvision;
EXTERN_C const CLSID CLSID_BigDriveService;

class BigDriveService
{
public:

	/// <summary>
	/// Creates and initializes a sample provider drive.
	/// </summary>
	/// <returns>Returns an HRESULT indicating success or failure of the operation.</returns>
	HRESULT CreateSampleProviderDrive();
};