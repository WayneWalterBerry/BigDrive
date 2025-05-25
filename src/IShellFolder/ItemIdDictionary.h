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
/// Object identifiers in the explorer's name space (ItemID and IDList)
///
///  All the items that the user can browse with the explorer (such as files,
/// directories, servers, work-groups, etc.) has an identifier which is unique
/// among items within the parent folder. Those identifiers are called item
/// IDs (SHITEMID). Since all its parent folders have their own item IDs,
/// any items can be uniquely identified by a list of item IDs, which is called
/// an ID list (ITEMIDLIST).
///
///  ID lists are almost always allocated by the task allocator (see some
/// description below as well as OLE 2.0 SDK) and may be passed across
/// some of shell interfaces (such as IShellFolder). Each item ID in an ID list
/// is only meaningful to its parent folder (which has generated it), and all
/// the clients must treat it as an opaque binary data except the first two
/// bytes, which indicates the size of the item ID.
///
///  When a shell extension -- which implements the IShellFolder interface --
/// generates an item ID, it may put any information in it, not only the data
/// with that it needs to identify the item, but also some additional
/// information, which would help implementing some other functions efficiently.
/// For example, the shell's IShellFolder implementation of file system items
/// stores the primary (long) name of a file or a directory as the item
/// identifier, but it also stores its alternative (short) name, size and date
/// etc.
///
///  When an ID list is passed to one of shell APIs (such as SHGetPathFromIDList),
/// it is always an absolute path -- relative from the root of the name space,
/// which is the desktop folder. When an ID list is passed to one of IShellFolder
/// member function, it is always a relative path from the folder (unless it
/// is explicitly specified).
///
/// ===========================================================================
///
/// SHITEMID -- Item ID  (mkid)
///     USHORT      cb;             // Size of the ID (including cb itself)
///     BYTE        abID[];         // The item ID (variable length)
///
/// ===========================================================================
/// </summary>

#pragma pack(push, 1)
struct BIGDRIVE_ITEMID {
	USHORT  cb;        // Size of this structure including cb
	LPCWSTR szName;    // Unique identifier
};
#pragma pack(pop)

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

	/// <summary>
	/// Serializes an ITEMIDLIST (LPITEMIDLIST) into a BSTR, converting each SHITEMID's abID to a hex string,
	/// separated by '/' characters.
	/// </summary>
	/// <param name="pidl">Pointer to the ITEMIDLIST to serialize.</param>
	/// <param name="bstPath">Reference to a BSTR that receives the resulting hex string.</param>
	/// <returns>S_OK on success, or an error HRESULT on failure.</returns>
	static HRESULT Serialize(_In_ LPCITEMIDLIST pidl, _Out_ BSTR& brstPath);

	/// <summary>
	/// Deserializes a BSTR produced by SerializeList into an ITEMIDLIST (LPITEMIDLIST).
	/// </summary>
	/// <param name="bstrPath">The BSTR hex string to deserialize.</param>
	/// <param name="ppidl">[out] Receives the resulting LPITEMIDLIST. Caller must free with CoTaskMemFree.</param>
	/// <returns>S_OK on success, or an error HRESULT on failure.</returns>
	static HRESULT Deserialize(_In_ const BSTR bstrPath, _Out_ LPITEMIDLIST* ppidl);


private:

	int HashPidl(LPCITEMIDLIST pidl) const;
	BOOL ComparePidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) const;
};