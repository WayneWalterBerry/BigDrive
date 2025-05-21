// <copyright file="ETWManifestManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <strsafe.h>
#include <iostream>
#include <aclapi.h>
#include <sddl.h>

#include "ETWManifestManager.h"
#include "LaunchDebugger.h"

BigDriveShellFolderEventLogger ETWManifestManager::s_eventLogger(L"BigDrive.ShellFolder");
extern "C" IMAGE_DOS_HEADER __ImageBase;

/// <inheritdoc />
HRESULT ETWManifestManager::GetManifestPath(LPWSTR* ppManifestPath)
{
	HRESULT hr = S_OK;
	WCHAR szModulePath[MAX_PATH] = { 0 };
	LPWSTR manifestPath = NULL;
	SIZE_T manifestPathLength = 0;
	SIZE_T dirPartLength = 0;
	DWORD fileAttributes;
	LPWSTR lastBackslash;

	if (ppManifestPath == NULL)
	{
		return E_POINTER;
	}

	// Initialize output parameter
	*ppManifestPath = NULL;

	// Get the path of the current DLL using __ImageBase
	DWORD pathLength = GetModuleFileNameW((HINSTANCE)&__ImageBase, szModulePath, MAX_PATH);
	if (pathLength == 0 || pathLength == MAX_PATH)
	{
		// Failed to get module path or buffer too small
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto End;
	}

	// Find the last backslash to extract the directory
	lastBackslash = wcsrchr(szModulePath, L'\\');
	if (lastBackslash == NULL)
	{
		hr = E_UNEXPECTED;
		goto End;
	}

	// Calculate length needed for the full manifest path
	// Add 1 for null terminator and +19 for "BigDriveEvents.man"
	manifestPathLength = MAX_PATH;

	// Allocate memory for the manifest path using CoTaskMemAlloc
	manifestPath = (LPWSTR)::CoTaskMemAlloc(manifestPathLength * sizeof(WCHAR));
	if (manifestPath == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

	// Copy directory part
	dirPartLength = lastBackslash + 1 - szModulePath;
	hr = StringCchCopyNW(manifestPath, manifestPathLength, szModulePath, dirPartLength);
	if (FAILED(hr))
	{
		goto End;
	}

	// Append filename
	hr = StringCchCatW(manifestPath, manifestPathLength, L"BigDriveEvents.man");
	if (FAILED(hr))
	{
		goto End;
	}

	// Verify the file exists
	fileAttributes = GetFileAttributesW(manifestPath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto End;
	}

	// Success, set the output parameter
	*ppManifestPath = manifestPath;
	manifestPath = NULL; // Ownership transferred

End:
	// Clean up if there was an error
	if (manifestPath != NULL)
	{
		::CoTaskMemFree(manifestPath);
	}

	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::BuildCommandLine(LPCWSTR action, LPCWSTR manifestPath, LPWSTR buffer, SIZE_T bufferSize)
{
	// Format the command line: wevtutil.exe [action] "[manifestPath]"
	HRESULT hr = StringCchPrintfW(buffer, bufferSize, L"wevtutil.exe %s \"%s\"", action, manifestPath);
	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::GrantEventLogServiceAccess()
{
	HRESULT hr = S_OK;
	WCHAR szModulePath[MAX_PATH] = { 0 };

	// Get the path of the current DLL using __ImageBase
	DWORD pathLength = GetModuleFileNameW((HINSTANCE)&__ImageBase, szModulePath, MAX_PATH);
	if (pathLength == 0 || pathLength == MAX_PATH)
	{
		// Failed to get module path or buffer too small
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto End;
	}

	bool bEventLogAccess;
	hr = CheckEventLogAccess(szModulePath, bEventLogAccess);
	if (FAILED(hr))
	{
		hr = E_ACCESSDENIED;
		goto End;
	}

	if (!bEventLogAccess)
	{
		// Grant access to the EventLog service
		hr = GrantEventLogAccess(szModulePath);
		if (FAILED(hr))
		{
			goto End;
		}
	}

End:

	return  hr;

}

/// <inheritdoc />
HRESULT ETWManifestManager::RegisterManifest()
{
	HRESULT hr = S_OK;
	LPWSTR szManifestPath = nullptr;
	WCHAR szModulePath[MAX_PATH] = { 0 };

	hr = GetManifestPath(&szManifestPath);
	if (FAILED(hr))
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to get manifest path", hr);
		return hr;
	}

	// The Event Log service (NT SERVICE\EventLog) needs read & execute access to the resource dll
	// to read the manifest and use the embedded resources.
	hr = GrantEventLogServiceAccess();
	if (FAILED(hr))
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to grant Event Log service access", hr);
		return hr;
	}

	// Get the path of the current DLL using __ImageBase
	DWORD pathLength = GetModuleFileNameW((HINSTANCE)&__ImageBase, szModulePath, MAX_PATH);
	if (pathLength == 0 || pathLength == MAX_PATH)
	{
		// Failed to get module path or buffer too small
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto End;
	}

    // Register the event provider for legacy event log support and event viewer scenarios
    hr = ETWManifestManager::RegisterEventLogProvider(L"BigDriveAnalytic", szModulePath);
	if (FAILED(hr))
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to register Event Log provider", hr);
		goto End;
	}

	// Register the manifest
	hr = RegisterManifest(szManifestPath);
	if (FAILED(hr))
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to register manifest", hr);
		goto End;
	}

	hr = VerifyManifestRegistration(szManifestPath);
	if (FAILED(hr))
	{
		s_eventLogger.WriteErrorFormmated(L"Failed to verify manifest registration", hr);	
		goto End;
	}

End:

	if (szManifestPath != nullptr)
	{
		// Free the manifest path memory
		::CoTaskMemFree(szManifestPath);
		szManifestPath = nullptr;
	}

	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::RegisterManifest(LPCWSTR manifestPath)
{
	// Maximum command line length (adjust if needed)
	const SIZE_T CMD_BUFFER_SIZE = 2048;
	WCHAR cmdLine[CMD_BUFFER_SIZE];

	// Build the command line for installation
	HRESULT hr = BuildCommandLine(L"im", manifestPath, cmdLine, CMD_BUFFER_SIZE);
	if (FAILED(hr))
	{
		// Failed to build command line (path too long)
		return hr;
	}

	return ExecuteWevtutil(cmdLine);
}

/// <inheritdoc />
HRESULT ETWManifestManager::UnregisterManifest()
{
	HRESULT hr = S_OK;
	LPWSTR manifestPath = nullptr;

	hr = GetManifestPath(&manifestPath);
	if (FAILED(hr))
	{
		return hr;
	}

	// Register the manifest
	hr = UnregisterManifest(manifestPath);
	if (FAILED(hr))
	{
		// Failed to register the manifest
		goto End;
	}

End:

	if (manifestPath != nullptr)
	{
		// Free the manifest path memory
		::CoTaskMemFree(manifestPath);
		manifestPath = nullptr;
	}

	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::UnregisterManifest(LPCWSTR manifestPath)
{
	// Maximum command line length (adjust if needed)
	const SIZE_T CMD_BUFFER_SIZE = 2048;
	WCHAR cmdLine[CMD_BUFFER_SIZE];

	// Build the command line for uninstallation
	HRESULT hr = BuildCommandLine(L"um", manifestPath, cmdLine, CMD_BUFFER_SIZE);
	if (FAILED(hr))
	{
		// Failed to build command line (path too long)
		return hr;
	}

	return ExecuteWevtutil(cmdLine);
}

/// <inheritdoc />
bool ETWManifestManager::wstr_contains(const WCHAR* haystack, const WCHAR* needle)
{
	if (!haystack || !needle || !*needle)
		return false;

	// Helper: skip whitespace and CR/LF
	auto skip_ws = [](const WCHAR*& p) {
		while (*p == L' ' || *p == L'\t' || *p == L'\r' || *p == L'\n')
			++p;
		};

	// Helper: compare haystack at h and needle at n, ignoring whitespace/CR/LF
	auto match_at = [&](const WCHAR* h, const WCHAR* n) -> bool {
		while (*n)
		{
			skip_ws(n);
			skip_ws(h);

			if (!*n)
				return true; // End of needle, match

			if (!*h)
				return false; // End of haystack, no match

			if (*h != *n)
				return false;

			++h;
			++n;
		}
		return true;
		};

	// For each position in haystack, try to match
	for (const WCHAR* h = haystack; *h; ++h)
	{
		const WCHAR* n = needle;
		if (*h == *n || (*n && (*h == *n || (*h == L' ' || *h == L'\t' || *h == L'\r' || *h == L'\n'))))
		{
			if (match_at(h, n))
				return true;
		}
	}
	return false;
}

/// <inheritdoc />
HRESULT ETWManifestManager::ExecuteWevtutil(LPCWSTR cmdLine)
{
	// Declare all local variables at the beginning
	HRESULT hr = S_OK;
	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION pi = {};
	SIZE_T bufferSize = 0;
	LPWSTR mutableCmdLine = nullptr;
	BOOL result = FALSE;
	DWORD exitCode = 0;
	HANDLE hStdOutRead = NULL;
	HANDLE hStdOutWrite = NULL;
	SECURITY_ATTRIBUTES sa = {};
	WCHAR* outputBuffer = nullptr;
	SIZE_T outputBufferSize = 0;
	SIZE_T outputBufferUsed = 0;
	CHAR buffer[4096];
	DWORD bytesRead = 0;

	// Check input for null
	if (cmdLine == nullptr)
	{
		hr = E_POINTER;
		goto End;
	}

	// Create a pipe for the child process's STDOUT.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if (!::CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		goto End;
	}

	// Ensure the read handle to the pipe is not inherited.
	if (!::SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0))
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		goto End;
	}

	// Set up STARTUPINFO to redirect stdout
	::ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = NULL;
	si.hStdOutput = hStdOutWrite;
	si.hStdError = hStdOutWrite;

	// Create a mutable copy of the command line
	bufferSize = ::lstrlenW(cmdLine) + 1;
	mutableCmdLine = new WCHAR[bufferSize];

	hr = ::StringCchCopyW(mutableCmdLine, bufferSize, cmdLine);
	if (FAILED(hr))
	{
		goto End;
	}

	// Create process to execute wevtutil
	result = ::CreateProcessW(
		NULL,                   // No application name (use command line)
		mutableCmdLine,         // Command line
		NULL,                   // Process handle not inheritable
		NULL,                   // Thread handle not inheritable
		TRUE,                   // Set handle inheritance to TRUE for redirection
		CREATE_NO_WINDOW,       // Don't create a window
		NULL,                   // Use parent's environment block
		NULL,                   // Use parent's starting directory
		&si,                    // Pointer to STARTUPINFO structure
		&pi                     // Pointer to PROCESS_INFORMATION structure
	);

	if (!result)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		goto End;
	}

	// Clean up the buffer as it's no longer needed
	if (mutableCmdLine != nullptr)
	{
		delete[] mutableCmdLine;
		mutableCmdLine = nullptr;
	}

	// Close the write end in the parent process immediately after CreateProcessW
	if (hStdOutWrite != NULL)
	{
		::CloseHandle(hStdOutWrite);
		hStdOutWrite = NULL;
	}

	// Allocate initial output buffer (4K WCHARs, can grow)
	outputBufferSize = 4096;
	outputBuffer = new WCHAR[outputBufferSize];
	outputBufferUsed = 0;
	outputBuffer[0] = L'\0';

	// Read output from the child process's stdout
	while (::ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0';

		// Convert to wide char
		int wcharsNeeded = ::MultiByteToWideChar(CP_ACP, 0, buffer, bytesRead, NULL, 0);
		if (wcharsNeeded > 0)
		{
			// Grow buffer if needed
			if (outputBufferUsed + wcharsNeeded + 1 > outputBufferSize)
			{
				SIZE_T newSize = outputBufferSize * 2 + wcharsNeeded + 1;
				WCHAR* newBuffer = new WCHAR[newSize];
				::memcpy(newBuffer, outputBuffer, outputBufferUsed * sizeof(WCHAR));
				delete[] outputBuffer;
				outputBuffer = newBuffer;
				outputBufferSize = newSize;
			}
			int written = ::MultiByteToWideChar(CP_ACP, 0, buffer, bytesRead, outputBuffer + outputBufferUsed, (int)(outputBufferSize - outputBufferUsed));
			if (written > 0)
			{
				outputBufferUsed += written;
				outputBuffer[outputBufferUsed] = L'\0';
			}
		}
	}

	// Wait for the process to finish
	::WaitForSingleObject(pi.hProcess, INFINITE);

	// Get the exit code
	::GetExitCodeProcess(pi.hProcess, &exitCode);

	// Return success if process exited with exit code 0, otherwise return error
	hr = (exitCode == 0) ? S_OK : E_FAIL;
	if (FAILED(hr))
	{
		// Log the output string for debugging
		s_eventLogger.WriteErrorFormmated(L"wevtutil command failed with output: %s", outputBuffer ? outputBuffer : L"(no output)");
		hr = E_FAIL;
		goto End;
	}

	// Check for specific warning in outputBuffer
	if (outputBuffer && wstr_contains(outputBuffer, L"**** Warning: The resource file for publisher BigDriveAnalytic was not found or could not be opened."))
	{
		s_eventLogger.WriteErrorFormmated(L"wevtutil.exe command failed: Failed to find resource file. Output: %s", outputBuffer);
		hr = E_FAIL;
		goto End;
	}

	if (outputBuffer && wstr_contains(outputBuffer, L"**** Warning: Publisher BigDriveAnalytic resources could not be found or are not accessible to the EventLog service account (NT SERVICE\\EventLog)."))
	{
		s_eventLogger.WriteErrorFormmated(L"wevtutil.exe command failed: Resources Not Found/Not Available. Output: %s", outputBuffer);
		hr = E_FAIL;
		goto End;
	}

End:

	// Clean up handles and memory
	if (mutableCmdLine != nullptr)
	{
		delete[] mutableCmdLine;
		mutableCmdLine = nullptr;
	}

	if (outputBuffer != nullptr)
	{
		delete[] outputBuffer;
		outputBuffer = nullptr;
	}

	if (hStdOutWrite != NULL)
	{
		::CloseHandle(hStdOutWrite);
		hStdOutWrite = NULL;
	}

	if (hStdOutRead != NULL)
	{
		::CloseHandle(hStdOutRead);
		hStdOutRead = NULL;
	}

	if (pi.hProcess != NULL)
	{
		::CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}

	if (pi.hThread != NULL)
	{
		::CloseHandle(pi.hThread);
		pi.hThread = NULL;
	}

	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::CheckEventLogAccess(LPCWSTR path, bool& bHasAccess)
{
	if (path == NULL)
	{
		return E_POINTER;
	}

	bHasAccess = FALSE;
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pACL = NULL;
	PSID pEventLogSID = NULL;
	HRESULT hr = S_OK;

	// Define file-specific access rights we require instead of generic rights
	// This is what GENERIC_READ | GENERIC_EXECUTE maps to for files
	ACCESS_MASK specificMask = FILE_GENERIC_READ | FILE_EXECUTE;

	// Get the security descriptor for the file
	DWORD result = ::GetNamedSecurityInfoW(path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pACL, NULL, &pSD);

	if (result != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(result);
		goto Cleanup;
	}

	hr = GetEventLogServiceSid(&pEventLogSID);
	if (FAILED(hr))
	{
		goto Cleanup;
	}

	// Check if the EventLog service has read/execute access

	for (WORD i = 0; i < pACL->AceCount; i++)
	{
		void* pACE;
		if (::GetAce(pACL, i, &pACE))
		{
			ACCESS_ALLOWED_ACE* ace = (ACCESS_ALLOWED_ACE*)pACE;

			LPWSTR lpszSid = NULL;
			::ConvertSidToStringSidW((PSID)&ace->SidStart, &lpszSid);
			::LocalFree(lpszSid);

			bool bEqualSid = ::EqualSid(pEventLogSID, (PSID)&ace->SidStart);

			// Check if all required access rights are included
			if (bEqualSid && ((ace->Mask & specificMask) == specificMask))
			{
				bHasAccess = TRUE;
				break;
			}
		}
	}

Cleanup:

	if (pSD != NULL)
	{
		::LocalFree(pSD);
		pSD = NULL;
	}

	if (pEventLogSID != NULL)
	{
		::LocalFree(pEventLogSID);
		pEventLogSID = NULL;
	}

	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::GrantEventLogAccess(LPCWSTR path)
{
	HRESULT hr = S_OK;

	if (path == NULL)
	{
		return E_POINTER;
	}

	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pOldACL = NULL;
	PACL pNewACL = NULL;
	PSID pEventLogSID = NULL;
	EXPLICIT_ACCESS ea = { 0 };
	DWORD dwRes = 0;

	hr = GetEventLogServiceSid(&pEventLogSID);
	if (FAILED(hr))
	{
		goto Cleanup;
	}

	// Get the existing DACL
	dwRes = GetNamedSecurityInfoW(
		path,
		SE_FILE_OBJECT,
		DACL_SECURITY_INFORMATION,
		NULL,
		NULL,
		&pOldACL,
		NULL,
		&pSD);

	if (dwRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(dwRes);
		goto Cleanup;
	}

	// Prepare the EXPLICIT_ACCESS structure for the new ACE
	ea.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPWSTR)pEventLogSID;

	// Create new ACL by merging the old one with our new entry
	dwRes = ::SetEntriesInAclW(1, &ea, pOldACL, &pNewACL);
	if (dwRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(dwRes);
		goto Cleanup;
	}

	// Apply the new ACL to the file
	dwRes = ::SetNamedSecurityInfoW(
		(LPWSTR)path,
		SE_FILE_OBJECT,
		DACL_SECURITY_INFORMATION,
		NULL,
		NULL,
		pNewACL,
		NULL);

	if (dwRes != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(dwRes);
		goto Cleanup;
	}

Cleanup:

	if (pSD != NULL)
	{
		LocalFree(pSD);
		pSD = NULL;
	}

	if (pNewACL != NULL)
	{
		LocalFree(pNewACL);
		pNewACL = NULL;
	}

	if (pEventLogSID != NULL)
	{
		LocalFree(pEventLogSID);
		pEventLogSID = NULL;
	}

	return hr;
}

/// <inheritdoc />
HRESULT ETWManifestManager::VerifyManifestRegistration(LPCWSTR manifestPath)
{
	if (manifestPath == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	HKEY hKey = NULL;
	WCHAR regPath[MAX_PATH + 50] = L"SYSTEM\\CurrentControlSet\\Control\\WMI\\Manifests\\";
	WCHAR manifestName[MAX_PATH] = { 0 };
	const WCHAR* lastBackslash = wcsrchr(manifestPath, L'\\');

	// Extract the manifest filename without path
	if (lastBackslash != NULL)
	{
		hr = StringCchCopyW(manifestName, MAX_PATH, lastBackslash + 1);
	}
	else
	{
		hr = StringCchCopyW(manifestName, MAX_PATH, manifestPath);
	}

	if (FAILED(hr))
	{
		return hr;
	}

	// Append manifest filename to registry path
	hr = StringCchCatW(regPath, MAX_PATH + 50, manifestName);
	if (FAILED(hr))
	{
		return hr;
	}

	// Open the registry key
	DWORD result = RegOpenKeyExW(
		HKEY_LOCAL_MACHINE,
		regPath,
		0,
		KEY_READ,
		&hKey);

	if (result != ERROR_SUCCESS)
	{
		// Registry key not found
		return HRESULT_FROM_WIN32(result);
	}

	// Check for required values
	DWORD valueType = 0;
	DWORD dataSize = 0;

	// Check for "ResourceFileName" value
	result = RegQueryValueExW(
		hKey,
		L"ResourceFileName",
		NULL,
		&valueType,
		NULL,
		&dataSize);

	if (result != ERROR_SUCCESS || valueType != REG_EXPAND_SZ)
	{
		RegCloseKey(hKey);
		return HRESULT_FROM_WIN32(result != ERROR_SUCCESS ? result : ERROR_INVALID_DATA);
	}

	// Check for "BinaryPath" value
	result = RegQueryValueExW(
		hKey,
		L"BinaryPath",
		NULL,
		&valueType,
		NULL,
		&dataSize);

	if (result != ERROR_SUCCESS || valueType != REG_EXPAND_SZ)
	{
		RegCloseKey(hKey);
		return HRESULT_FROM_WIN32(result != ERROR_SUCCESS ? result : ERROR_INVALID_DATA);
	}

	// Cleanup
	RegCloseKey(hKey);

	return S_OK;
}

/// <inheritdoc />
HRESULT ETWManifestManager::GetEventLogServiceSid(PSID* ppSid)
{
	HRESULT hr = S_OK;

	DWORD cbSid = 0;
	DWORD cchReferencedDomainName = 0;
	SID_NAME_USE eUse;
	WCHAR szDomainName[MAX_PATH] = { 0 };

	// First call to get required buffer sizes
	LookupAccountNameW(
		NULL,                       // local machine
		L"NT SERVICE\\EventLog",    // account name
		NULL,                       // Sid buffer
		&cbSid,                     // size of Sid buffer
		szDomainName,               // domain buffer
		&cchReferencedDomainName,   // size of domain buffer
		&eUse                       // SID_NAME_USE enum
	);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Allocate required buffers
	*ppSid = (PSID)LocalAlloc(LPTR, cbSid);
	if (*ppSid == NULL)
	{
		return E_OUTOFMEMORY;
	}

	// Second call to get the actual SID
	if (!LookupAccountNameW(
		NULL,                      // local machine
		L"NT SERVICE\\EventLog",   // account name
		*ppSid,                      // Sid buffer
		&cbSid,                    // size of Sid buffer
		szDomainName,              // domain buffer
		&cchReferencedDomainName,  // size of domain buffer
		&eUse))                    // SID_NAME_USE enum
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LocalFree(*ppSid);
		*ppSid = NULL;
		return hr;
	}

	return S_OK;
}

/// <inheritdoc />
HRESULT ETWManifestManager::RegisterEventLogProvider(LPCWSTR providerName, LPCWSTR messageFilePath)
{
	HRESULT hr = S_OK;
	HKEY hKey = NULL;
	WCHAR regPath[MAX_PATH * 2] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
	LONG lResult;
	DWORD typesSupported = 0x7;

	if (!providerName || !*providerName || !messageFilePath || !*messageFilePath)
	{
		hr = E_INVALIDARG;
		goto End;
	}

	// Build the registry path for the provider
	hr = ::StringCchCatW(regPath, ARRAYSIZE(regPath), providerName);
	if (FAILED(hr))
	{
		goto End;
	}

	// Create or open the provider key
	lResult = ::RegCreateKeyExW(
		HKEY_LOCAL_MACHINE,
		regPath,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		NULL,
		&hKey,
		NULL);

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

	// Set the EventMessageFile value (REG_EXPAND_SZ)
	lResult = ::RegSetValueExW(
		hKey,
		L"EventMessageFile",
		0,
		REG_EXPAND_SZ,
		(const BYTE*)messageFilePath,
		(DWORD)((::lstrlenW(messageFilePath) + 1) * sizeof(WCHAR)));

	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

	// Set the TypesSupported value (REG_DWORD, typically 7 = Error|Warning|Information)
	lResult = ::RegSetValueExW(
		hKey,
		L"TypesSupported",
		0,
		REG_DWORD,
		(const BYTE*)&typesSupported,
		sizeof(typesSupported));
	if (lResult != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lResult);
		goto End;
	}

End:

	if (hKey != NULL)
	{
		::RegCloseKey(hKey);
		hKey = NULL;
	}

	return hr;
}