// <copyright file="ItemIdDictionary.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>
// <summary>
//   Minimal dictionary implementation mapping ITEMID (PIDL) keys to LPWSTR values.
//   This implementation does not use the C++ Standard Library and is suitable for
//   use in shell extensions where custom memory management and PIDL comparison are required.
//   The dictionary supports insertion, lookup, and removal of key-value pairs.
//   All methods return HRESULTs and use output parameters for results.
// </summary>

#pragma once

#include "pch.h"

#include "ItemIdDictionaryStaticInitializer.h"

#include <windows.h>
#include <shlobj.h>



/// <summary>
/// Represents a dictionary entry mapping an ITEMID (PIDL) to an LPWSTR value.
/// </summary>
struct ItemIdDictionaryEntry
{
	LPITEMIDLIST key;   // The PIDL key (cloned on insert, freed on remove)
	LPWSTR value;       // The value (allocated and freed by the dictionary)
	ItemIdDictionaryEntry* next;
};

/// <summary>
/// A minimal dictionary mapping ITEMID (PIDL) keys to LPWSTR values.
/// Uses a simple hash table with separate chaining for collision resolution.
/// All methods return HRESULTs and use output parameters for results.
/// </summary>
class ItemIdDictionary
{
private:

	/// <summary>
	/// Prime number for hash table size
	/// </summary>
	static const int TABLE_SIZE = 37;

	/// <summary>
	/// Declares a static initializer object for the ItemIdDictionary.
	/// </summary>
	static ItemIdDictionaryStaticInitializer s_staticInitializer;

private:

	/// <summary>
	/// An array of pointers to ItemIdDictionaryEntry objects, representing the buckets of a hash table.
	/// </summary>
	ItemIdDictionaryEntry* m_buckets[TABLE_SIZE];

public:

	ItemIdDictionary();

	~ItemIdDictionary();

	/// <summary>
	/// Inserts or updates a key-value pair in the dictionary.
	/// The key is cloned and the value is copied.
	/// </summary>
	/// <param name="key">The PIDL key (will be cloned).</param>
	/// <param name="value">The string value (will be copied).</param>
	/// <returns>S_OK if inserted/updated, error HRESULT on failure.</returns>
	HRESULT Insert(LPCITEMIDLIST key, LPCWSTR value);

	/// <summary>
	/// Looks up a value by its PIDL key.
	/// </summary>
	/// <param name="key">The PIDL key to look up.</param>
	/// <param name="outValue">Receives the value pointer, or nullptr if not found.</param>
	/// <returns>S_OK if found, S_FALSE if not found, error HRESULT on failure.</returns>
	HRESULT Lookup(LPCITEMIDLIST key, LPCWSTR* outValue) const;

	/// <summary>
	/// Removes a key-value pair by its PIDL key.
	/// </summary>
	/// <param name="key">The PIDL key to remove.</param>
	/// <returns>S_OK if removed, S_FALSE if not found, error HRESULT on failure.</returns>
	HRESULT Remove(LPCITEMIDLIST key);

	/// <summary>
	/// Removes all entries and frees memory.
	/// </summary>
	HRESULT Clear();

private:

	int HashPidl(LPCITEMIDLIST pidl) const;
	BOOL ComparePidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) const;
};