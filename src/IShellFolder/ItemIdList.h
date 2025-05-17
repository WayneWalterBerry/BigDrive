// <copyright file="ItemIdList.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <windows.h>
#include <shtypes.h>

class __declspec(dllexport) ItemIdList
{
private:

    LPITEMIDLIST m_pidl;

public:

    ItemIdList(LPITEMIDLIST pidl)
        : m_pidl(pidl)
    {
    }

    /// <summary>
    /// Retrieves the next ITEMID in the list.
    /// </summary>
    /// <param name="next">[out] Receives the next ItemIdList if available; otherwise, an empty ItemIdList.</param>
    /// <returns>S_OK if the next item exists, S_FALSE if at the end of the list, or E_POINTER if m_pidl is null.</returns>
    HRESULT NextItem(ItemIdList& next) const;

    /// <summary>
    /// Serializes an ITEMIDLIST (LPITEMIDLIST) into a BSTR, converting each SHITEMID's abID to a hex string,
    /// separated by '/' characters.
    /// </summary>
    /// <param name="pidl">Pointer to the ITEMIDLIST to serialize.</param>
    /// <param name="bstPath">Reference to a BSTR that receives the resulting hex string.</param>
    /// <returns>S_OK on success, or an error HRESULT on failure.</returns>
    HRESULT SerializeList(_Out_ BSTR& brstPath);

    /// <summary>
    /// Deserializes a BSTR produced by SerializeList into an ITEMIDLIST (LPITEMIDLIST).
    /// </summary>
    /// <param name="bstrPath">The BSTR hex string to deserialize.</param>
    /// <param name="ppidl">[out] Receives the resulting LPITEMIDLIST. Caller must free with CoTaskMemFree.</param>
    /// <returns>S_OK on success, or an error HRESULT on failure.</returns>
    static HRESULT DeserializeList(_In_ BSTR bstrPath, _Out_ LPITEMIDLIST* ppidl);
};