// <copyright file="TrustedInstaller.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

/// <summary>
/// Provides static helper methods for impersonating the TrustedInstaller account and related operations.
/// </summary>
class TrustedInstaller
{
public:

	/// <summary>
	/// Impersonates the TrustedInstaller account for the current thread.
	/// </summary>
	/// <param name="hImpersonationToken">[out] Receives the duplicated impersonation token. Caller must call Revert and CloseHandle.</param>
	/// <returns>S_OK on success, or an HRESULT error code on failure.</returns>
	static HRESULT Impersonate(HANDLE* hImpersonationToken);

	/// <summary>
	/// Reverts the current thread from TrustedInstaller impersonation and closes the impersonation token.
	/// </summary>
	/// <param name="hImpersonationToken">The impersonation token returned by Impersonate. This function will close the handle.</param>
	/// <returns>S_OK on success, or an HRESULT error code on failure.</returns>
	static HRESULT Revert(HANDLE hImpersonationToken);

private:

	/// <summary>
	/// Starts the TrustedInstaller service if it is not already running and retrieves its process ID.
	/// </summary>
	/// <param name="tiPid">[out] Receives the TrustedInstaller process ID if successful.</param>
	/// <returns>S_OK on success, or an HRESULT error code on failure.</returns>
	static HRESULT StartService(DWORD* tiPid);


	/// <summary>
	/// Retrieves a handle to the TrustedInstaller process.
	/// </summary>
	/// <param name="tiPid">The process ID of TrustedInstaller.</param>
	/// <param name="hProcess">[out] Receives the process handle. Caller must CloseHandle.</param>
	/// <returns>S_OK on success, or an HRESULT error code on failure.</returns>
	static HRESULT GetProcessHandle(DWORD tiPid, HANDLE* hProcess);

};
