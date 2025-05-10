// <copyright file="BigDriveShellFolder.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolder.h"

/// <summary>
/// Queries the object for a pointer to one of its supported interfaces.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::QueryInterface(REFIID riid, void** ppvObject)
{
    if (riid == IID_IUnknown || riid == IID_IShellFolder)
    {
        *ppvObject = static_cast<IShellFolder*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

/// <summary>
/// Increments the reference count for the object.
/// </summary>
ULONG __stdcall BigDriveShellFolder::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

/// <summary>
/// Decrements the reference count for the object. Deletes the object if the reference count reaches zero.
/// </summary>
ULONG __stdcall BigDriveShellFolder::Release()
{
    LONG ref = InterlockedDecrement(&m_refCount);
    if (ref == 0)
    {
        delete this;
    }
    return ref;
}

/// <summary>
/// Parses a display name and returns a PIDL (Pointer to an Item ID List) that identifies the item.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
    ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Enumerates the objects in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList** ppenumIDList)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Binds to a specified object in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Binds to the storage of a specified object in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Compares two item IDs to determine their relative order.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Creates a view object for the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::CreateViewObject(HWND hwndOwner, REFIID riid, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Retrieves the attributes of one or more items in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Retrieves an object that can be used to carry out actions on the specified items.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
    REFIID riid, UINT* rgfReserved, void** ppv)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Retrieves the display name of an item in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName)
{
    // Placeholder implementation
    return E_NOTIMPL;
}

/// <summary>
/// Sets the display name of an item in the folder.
/// </summary>
HRESULT __stdcall BigDriveShellFolder::SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName,
    SHGDNF uFlags, PITEMID_CHILD* ppidlOut)
{
    // Placeholder implementation
    return E_NOTIMPL;
}
