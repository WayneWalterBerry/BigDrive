// <copyright file="ItemIdDictionaryExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shlobj.h>
#include <oleauto.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Opaque handle for ItemIdDictionary instance
	typedef void* HIDITEMIDDIC;

	// Creation and destruction
	__declspec(dllexport) HIDITEMIDDIC __stdcall ItemIdDictionary_Create();
	__declspec(dllexport) void __stdcall ItemIdDictionary_Destroy(HIDITEMIDDIC dict);

	// Dictionary operations
	__declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Insert(HIDITEMIDDIC dict, LPCITEMIDLIST key, LPCWSTR value);
	__declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Lookup(HIDITEMIDDIC dict, LPCITEMIDLIST key, LPCWSTR* outValue);
	__declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Remove(HIDITEMIDDIC dict, LPCITEMIDLIST key);
	__declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Clear(HIDITEMIDDIC dict);

	// Static serialization utilities
	__declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Serialize(LPCITEMIDLIST pidl, BSTR* bstrPath);
	__declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Deserialize(const BSTR bstrPath, LPITEMIDLIST* ppidl);

#ifdef __cplusplus
}
#endif