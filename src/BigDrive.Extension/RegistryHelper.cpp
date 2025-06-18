// <copyright file="RegistrationHelper.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// System
#include <windows.h>
#include <debugapi.h>   
#include <objbase.h>
#include <shobjidl.h>
#include <Aclapi.h>

#include "Logging\BigDriveTraceLogger.h"

HRESULT GetCurrentProcessSID(PSID* pOwner)
{
	HRESULT hr = S_OK;
	HANDLE hToken = nullptr;
	DWORD dwLen = 0;
	PTOKEN_USER pTokenUser = nullptr;
	DWORD sidLen = 0;

	if (!pOwner)
	{
		hr = E_POINTER;
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"GetCurrentProcessSID: pOwner pointer is null.");
		goto End;
	}

	*pOwner = nullptr;

	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"GetCurrentProcessSID: OpenProcessToken failed. Error: %u", ::GetLastError());
		goto End;
	}

	// First call to get required buffer size
	::GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLen);
	pTokenUser = (PTOKEN_USER)::malloc(dwLen);
	if (!pTokenUser)
	{
		hr = E_OUTOFMEMORY;
		BigDriveTraceLogger::LogEvent(__FUNCTION__, L"GetCurrentProcessSID: Failed to allocate memory for TOKEN_USER.");
		goto End;
	}

	if (!::GetTokenInformation(hToken, TokenUser, pTokenUser, dwLen, &dwLen))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"GetCurrentProcessSID: GetTokenInformation failed. Error: %u", ::GetLastError());
		goto End;
	}

	// Duplicate the SID for the caller
	sidLen = ::GetLengthSid(pTokenUser->User.Sid);
	*pOwner = (PSID)::malloc(sidLen);
	if (!*pOwner)
	{
		hr = E_OUTOFMEMORY;
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"GetCurrentProcessSID: Failed to allocate memory for SID.");
		goto End;
	}

	if (!::CopySid(sidLen, *pOwner, pTokenUser->User.Sid))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"GetCurrentProcessSID: CopySid failed. Error: %u", ::GetLastError());
		::free(*pOwner);
		*pOwner = nullptr;
		goto End;
	}

End:

	if (pTokenUser)
	{
		::free(pTokenUser);
		pTokenUser = nullptr;
	}

	if (hToken)
	{
		::CloseHandle(hToken);
		hToken = nullptr;
	}

	return hr;
}

/// </ inheritdoc>
HRESULT EnablePrivilege(LPCWSTR privilege)
{
	HRESULT hr = S_OK;
	HANDLE hToken = nullptr;
	TOKEN_PRIVILEGES tp = {};
	LUID luid = {};

	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"EnablePrivilege: OpenProcessToken failed. Error: %u", ::GetLastError());
		goto End;
	}

	if (!::LookupPrivilegeValueW(nullptr, privilege, &luid))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"EnablePrivilege: LookupPrivilegeValueW failed. Error: %u", ::GetLastError());
		goto End;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"EnablePrivilege: AdjustTokenPrivileges failed. Error: %u", ::GetLastError());
		goto End;
	}

	// AdjustTokenPrivileges may succeed but not enable the privilege
	if (::GetLastError() != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"EnablePrivilege: AdjustTokenPrivileges did not enable privilege. Error: %u", ::GetLastError());
		goto End;
	}

End:

	if (hToken)
	{
		::CloseHandle(hToken);
		hToken = nullptr;
	}

	return hr;
}

/// </ inheritdoc>
HRESULT TakeOwnershipAndGrantFullControl(LPCWSTR keyPath, HRESULT(*callback)())
{
	HRESULT hr = S_OK;
	HKEY hKey = nullptr;
	LONG lRes = 0;
	PSECURITY_DESCRIPTOR pOrigOwnerSD = nullptr;
	DWORD origOwnerSDSize = 0;
	PSECURITY_DESCRIPTOR pOrigDaclSD = nullptr;
	DWORD origDaclSDSize = 0;
	DWORD sdSize = 0;
	PSECURITY_DESCRIPTOR pSD = nullptr;
	BOOL bOwnerDefaulted = FALSE;
	BOOL isOwner = FALSE;
	SECURITY_DESCRIPTOR sd;
	EXPLICIT_ACCESSW ea = {};
	PACL pOldDACL = nullptr;
	PACL pNewDACL = nullptr;
	DWORD dwRes = 0;
	SECURITY_DESCRIPTOR sdNew;
	PSID psidCurrentUser = nullptr;
	BOOL ownerDefaulted = FALSE;
	PSID pOwner = nullptr;

	if (!callback)
	{
		hr = E_POINTER;
		BigDriveTraceLogger::LogEvent(__FUNCTION__, L"callback pointer is null.");
		goto End;
	}

	// 1. Open with READ_CONTROL to read OWNER and DACL
	lRes = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, READ_CONTROL, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to open registry key '%s' for READ_CONTROL. Error: %u", keyPath, lRes);
		goto End;
	}

	// Save original OWNER
	lRes = ::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, nullptr, &origOwnerSDSize);
	if (lRes != ERROR_INSUFFICIENT_BUFFER)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get size for OWNER_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	pOrigOwnerSD = (PSECURITY_DESCRIPTOR)::malloc(origOwnerSDSize);
	if (!pOrigOwnerSD)
	{
		hr = E_OUTOFMEMORY;
		BigDriveTraceLogger::LogEvent(__FUNCTION__, L"Failed to allocate memory for original owner security descriptor.");
		goto End;
	}

	lRes = ::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, pOrigOwnerSD, &origOwnerSDSize);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get OWNER_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	// Save original DACL
	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, nullptr, &origDaclSDSize);
	if (lRes != ERROR_INSUFFICIENT_BUFFER)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get size for DACL_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	pOrigDaclSD = (PSECURITY_DESCRIPTOR)::malloc(origDaclSDSize);
	if (!pOrigDaclSD)
	{
		hr = E_OUTOFMEMORY;
		BigDriveTraceLogger::LogEvent(__FUNCTION__, L"Failed to allocate memory for original DACL security descriptor.");
		goto End;
	}

	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pOrigDaclSD, &origDaclSDSize);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get DACL_SECURITY_INFORMATION. Error: %u", lRes);
		goto End;
	}

	::RegCloseKey(hKey);
	hKey = nullptr;

	// 2. Take ownership if needed (open with WRITE_OWNER)
	hr = GetCurrentProcessSID(&psidCurrentUser);
	if (FAILED(hr))
	{
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get current process SID. HRESULT: 0x%08X", hr);
		goto End;
	}

	// Check if current user is already owner
	if (::GetSecurityDescriptorOwner(pOrigOwnerSD, &pOwner, &ownerDefaulted) && pOwner && psidCurrentUser && ::EqualSid(pOwner, psidCurrentUser))
	{
		isOwner = TRUE;
	}

	if (!isOwner)
	{
		hr = EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
		if (FAILED(hr))
		{
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to enable SeTakeOwnershipPrivilege. HRESULT: 0x%08X", hr);
			goto End;
		}

		hr = EnablePrivilege(SE_RESTORE_NAME);
		if (FAILED(hr))
		{
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to enable SeTakeOwnershipPrivilege. HRESULT: 0x%08X", hr);
			goto End;
		}

		lRes = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, WRITE_OWNER | READ_CONTROL, &hKey);
		if (lRes != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lRes);
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to open registry key '%s' for WRITE_OWNER. Error: %u", keyPath, lRes);
			goto End;
		}

		if (!::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		{
			hr = HRESULT_FROM_WIN32(::GetLastError());
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to initialize security descriptor. Error: %u", ::GetLastError());
			goto End;
		}

		if (!::SetSecurityDescriptorOwner(&sd, psidCurrentUser, FALSE))
		{
			hr = HRESULT_FROM_WIN32(::GetLastError());
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to set security descriptor owner. Error: %u", ::GetLastError());
			goto End;
		}

		lRes = ::RegSetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, &sd);
		if (lRes != ERROR_SUCCESS)
		{
			hr = HRESULT_FROM_WIN32(lRes);
			BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to set key owner. Error: %u", lRes);
			goto End;
		}

		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	// 3. Grant full control (open with WRITE_DAC)
	lRes = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, WRITE_DAC | READ_CONTROL, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to open registry key '%s' for WRITE_DAC. Error: %u", keyPath, lRes);
		goto End;
	}

	ea.grfAccessPermissions = KEY_ALL_ACCESS;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea.Trustee.ptstrName = (LPWSTR)psidCurrentUser;

	// Get current DACL
	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, nullptr, &sdSize);
	if (lRes != ERROR_INSUFFICIENT_BUFFER)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get size for DACL_SECURITY_INFORMATION (WRITE_DAC). Error: %u", lRes);
		goto End;
	}

	pSD = (PSECURITY_DESCRIPTOR)::malloc(sdSize);
	if (!pSD)
	{
		hr = E_OUTOFMEMORY;
		BigDriveTraceLogger::LogEvent(__FUNCTION__, L"Failed to allocate memory for DACL security descriptor.");
		goto End;
	}

	lRes = ::RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pSD, &sdSize);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get DACL_SECURITY_INFORMATION (WRITE_DAC). Error: %u", lRes);
		goto End;
	}

	if (!::GetSecurityDescriptorDacl(pSD, &bOwnerDefaulted, &pOldDACL, &bOwnerDefaulted))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to get security descriptor DACL. Error: %u", ::GetLastError());
		goto End;
	}

	dwRes = ::SetEntriesInAclW(1, &ea, pOldDACL, &pNewDACL);
	if (dwRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(dwRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to set entries in ACL. Error: %u", dwRes);
		goto End;
	}

	if (!::InitializeSecurityDescriptor(&sdNew, SECURITY_DESCRIPTOR_REVISION))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to initialize new security descriptor. Error: %u", ::GetLastError());
		goto End;
	}

	if (!::SetSecurityDescriptorDacl(&sdNew, TRUE, pNewDACL, FALSE))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to set new security descriptor DACL. Error: %u", ::GetLastError());
		goto End;
	}

	lRes = ::RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION, &sdNew);
	if (lRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRes);
		BigDriveTraceLogger::LogErrorFormatted(__FUNCTION__, L"Failed to set key DACL. Error: %u", lRes);
		goto End;
	}

	// 4. Call the callback
	hr = callback();

	// 5. Restore original OWNER and DACL
	::RegSetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, pOrigOwnerSD);
	::RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pOrigDaclSD);

End:

	if (pNewDACL)
	{
		::LocalFree(pNewDACL);
		pNewDACL = nullptr;
	}

	if (pSD)
	{
		::free(pSD);
		pSD = nullptr;
	}

	if (psidCurrentUser)
	{
		::free(psidCurrentUser);
		psidCurrentUser = nullptr;
	}

	if (pOrigOwnerSD)
	{
		::free(pOrigOwnerSD);
		pOrigOwnerSD = nullptr;
	}

	if (pOrigDaclSD)
	{
		::free(pOrigDaclSD);
		pOrigDaclSD = nullptr;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = nullptr;
	}

	return hr;
}

