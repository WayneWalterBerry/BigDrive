// <copyright file="BigDriveShellFolderStatic.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <shlobj.h>

/// <summary>
/// Class to hold statics that are not constant and need to be initialized.
/// </summary>
class BigDriveShellFolderStatic
{
private:

    PIDLIST_ABSOLUTE s_pidlMyComputer;
    PWSTR pszName;

public:

	BigDriveShellFolderStatic()
	{
		::SHGetKnownFolderIDList(FOLDERID_ComputerFolder, 0, nullptr, &s_pidlMyComputer);
        ::SHGetNameFromIDList(s_pidlMyComputer, SIGDN_NORMALDISPLAY, &pszName);
	}

	~BigDriveShellFolderStatic()
	{
		if (s_pidlMyComputer != nullptr)
		{
			::CoTaskMemFree(s_pidlMyComputer);
			s_pidlMyComputer = nullptr;
		}

        if (pszName != nullptr)
        {
            ::CoTaskMemFree(pszName);
            pszName = nullptr;
        }
	}

    /// <summary>
    /// Determines if the given PIDL is an absolute PIDL rooted at "This PC" (My Computer).
    /// </summary>
    /// <param name="pidl">The PIDL to check.</param>
    /// <returns>true if the first ITEMID matches s_pidlMyComputer; false otherwise.</returns>
    bool IsPidlRootedAtMyComputer(LPCITEMIDLIST pidl)
    {
        if (!pidl || !s_pidlMyComputer)
        {
            return false;
        }

        // Compare the first ITEMID in both PIDLs
        const BYTE* p1 = reinterpret_cast<const BYTE*>(pidl);
        const BYTE* p2 = reinterpret_cast<const BYTE*>(s_pidlMyComputer);

        USHORT cb1 = *(const USHORT*)p1;
        USHORT cb2 = *(const USHORT*)p2;

        if (cb1 == 0 || cb2 == 0)
        {
            return false;
        }

        // Compare the first ITEMID (length and data)
        if (cb1 != cb2)
        {
            return false;
        }

        return (memcmp(p1, p2, cb1) == 0);
    }

    /// <summary>
    /// Retrieves the computer name as a wide string.
    /// </summary>
    /// <returns>A pointer to a wide string (LPWSTR) containing the computer name.</returns>
    LPCWSTR GetMyComputerName()
    {
        return pszName;
	}
};
