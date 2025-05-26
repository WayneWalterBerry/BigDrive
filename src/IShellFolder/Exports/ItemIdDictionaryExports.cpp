// <copyright file="ItemIdDictionaryExports.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "ItemIdDictionaryExports.h"

#include "../ItemIdDictionary.h"

extern "C" {

    // Opaque handle implementation
    typedef ItemIdDictionary* ItemIdDictionaryHandle;

    __declspec(dllexport) HIDITEMIDDIC __stdcall ItemIdDictionary_Create() {
        return reinterpret_cast<HIDITEMIDDIC>(new ItemIdDictionary());
    }

    __declspec(dllexport) void __stdcall ItemIdDictionary_Destroy(HIDITEMIDDIC dict) {
        if (dict) {
            delete reinterpret_cast<ItemIdDictionaryHandle>(dict);
        }
    }

    __declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Insert(HIDITEMIDDIC dict, LPCITEMIDLIST key, LPCWSTR value) {
        if (!dict) return E_POINTER;
        return reinterpret_cast<ItemIdDictionaryHandle>(dict)->Insert(key, value);
    }

    __declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Lookup(HIDITEMIDDIC dict, LPCITEMIDLIST key, LPCWSTR* outValue) {
        if (!dict) return E_POINTER;
        return reinterpret_cast<ItemIdDictionaryHandle>(dict)->Lookup(key, outValue);
    }

    __declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Remove(HIDITEMIDDIC dict, LPCITEMIDLIST key) {
        if (!dict) return E_POINTER;
        return reinterpret_cast<ItemIdDictionaryHandle>(dict)->Remove(key);
    }

    __declspec(dllexport) HRESULT __stdcall ItemIdDictionary_Clear(HIDITEMIDDIC dict) {
        if (!dict) return E_POINTER;
        return reinterpret_cast<ItemIdDictionaryHandle>(dict)->Clear();
    }

} // extern "C"