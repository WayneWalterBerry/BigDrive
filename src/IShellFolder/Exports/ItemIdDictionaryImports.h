// <copyright file="ItemIdDictionaryImprts.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shlobj.h>
#include <oleauto.h>

// Expose ItemIdDictionary public methods for interop or dynamic loading.
// This header is intended for use in scenarios similar to RegistrationManagerImports.h.

#ifdef __cplusplus
extern "C" {
#endif

	// Opaque handle for ItemIdDictionary instance
	typedef void* HIDITEMIDDIC;

	// Creation and destruction
	HIDITEMIDDIC __stdcall ItemIdDictionary_Create();
	void __stdcall ItemIdDictionary_Destroy(HIDITEMIDDIC dict);

	// Dictionary operations
	HRESULT __stdcall ItemIdDictionary_Insert(HIDITEMIDDIC dict, LPCITEMIDLIST key, LPCWSTR value);
	HRESULT __stdcall ItemIdDictionary_Lookup(HIDITEMIDDIC dict, LPCITEMIDLIST key, LPCWSTR* outValue);
	HRESULT __stdcall ItemIdDictionary_Remove(HIDITEMIDDIC dict, LPCITEMIDLIST key);
	HRESULT __stdcall ItemIdDictionary_Clear(HIDITEMIDDIC dict);

#ifdef __cplusplus
}
#endif