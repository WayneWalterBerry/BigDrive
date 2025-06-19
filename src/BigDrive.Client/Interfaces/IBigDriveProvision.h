// <copyright file="IBigDriveProvision.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <Unknwn.h>

/// <summary>
/// IBigDriveProvision COM interface for drive provisioning.
/// </summary>
struct IBigDriveProvision : public IUnknown
{
	/// <summary>
	/// Create a new drive with the specified GUID.
	/// </summary>
	/// <param name="driveGuid">The GUID of the drive to create.</param>
	/// <returns>HRESULT indicating success or failure.</returns>
	virtual STDMETHODIMP Create(/* [in] */ REFGUID driveGuid) = 0;

	/// <summary>
	/// Create a new drive with the specified JSON configuration.
	/// </summary>
	/// <param name="jsonConfiguration">JSON string describing the drive configuration.</param>
	/// <returns>HRESULT indicating success or failure.</returns>
	virtual STDMETHODIMP CreateFromConfiguration(/* [in] */ LPCWSTR jsonConfiguration) = 0;
};
