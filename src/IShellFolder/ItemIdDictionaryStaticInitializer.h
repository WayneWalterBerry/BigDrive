// <copyright file="ItemIdDictionaryStaticInitializer.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>
// <author>Wayne Walter Berry</author>

#pragma once

#include <shlobj.h>

/// <summary>
/// A static initializer class for the ItemIdDictionary.
/// Provides methods to retrieve special PIDLs, such as the drives folder.
/// </summary>
class ItemIdDictionaryStaticInitializer
{
private:
    LPITEMIDLIST m_drivesPidl;

public:

    ItemIdDictionaryStaticInitializer()
        : m_drivesPidl(nullptr)
    {
        // Initialize the drives PIDL
        ::SHGetSpecialFolderLocation(nullptr, CSIDL_DRIVES, &m_drivesPidl);
    }

    ~ItemIdDictionaryStaticInitializer()
    {
        if (m_drivesPidl)
        {
            ILFree(m_drivesPidl);
        }
    }

    /// <summary>
    /// Retrieves the PIDL for the drives folder.
    /// </summary>
    /// <returns>A constant pointer to the drives PIDL.</returns>
    LPCITEMIDLIST GetDrivesPidl() const
    {
        return m_drivesPidl;
    }
};