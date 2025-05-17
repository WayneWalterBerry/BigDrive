// <copyright file="ILExtensions.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shtypes.h>

/// <summary>
/// Serializes an ITEMIDLIST (LPITEMIDLIST) into a BSTR, converting each SHITEMID's abID to a hex string,
/// separated by '/' characters.
/// </summary>
/// <param name="pidl">Pointer to the ITEMIDLIST to serialize.</param>
/// <param name="bstPath">Reference to a BSTR that receives the resulting hex string.</param>
/// <returns>S_OK on success, or an error HRESULT on failure.</returns>
extern "C" HRESULT __stdcall ILSerialize(_In_ LPCITEMIDLIST pidl, _Out_ BSTR& brstPath);

/// <summary>
/// Deserializes a BSTR produced by SerializeList into an ITEMIDLIST (LPITEMIDLIST).
/// </summary>
/// <param name="bstrPath">The BSTR hex string to deserialize.</param>
/// <param name="ppidl">[out] Receives the resulting LPITEMIDLIST. Caller must free with CoTaskMemFree.</param>
/// <returns>S_OK on success, or an error HRESULT on failure.</returns>
extern "C" HRESULT __stdcall ILDeserialize(_In_ const BSTR bstrPath, _Out_ LPITEMIDLIST* ppidl);
