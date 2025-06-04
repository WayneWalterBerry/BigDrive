// <copyright file="BigDriveShellContextMenu.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include "BigDriveShellContextMenu.h"
#include "Logging\BigDriveShellFolderTraceLogger.h"

#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj_core.h>
#include <shobjidl.h>  // This header contains DEFCONTEXTMENU definitions

// Define this if it's not already defined - this is the actual value from the Windows SDK
#ifndef DEFCONTEXTMENU_CALLBACK_DATAOBJECT
#define DEFCONTEXTMENU_CALLBACK_DATAOBJECT 1
#endif

/// <summary>
/// Adds menu items to a context menu.
/// </summary>
IFACEMETHODIMP BigDriveShellContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    // Save the first command ID for use in other methods
    m_idCmdFirst = idCmdFirst;

    // If we're dealing with a verb-only request, handle minimal set
    if (uFlags & CMF_VERBSONLY)
    {
        // Add just the Open verb
        InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, idCmdFirst + ID_OPEN, L"&Open");
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);
    }

    // Get item type to customize the menu
    UINT basicCmdCount = 0;

    if (m_cidl == 1 && m_apidl && m_apidl[0])
    {
        // Get the item type
        const BIGDRIVE_ITEMID* pItemId = reinterpret_cast<const BIGDRIVE_ITEMID*>(m_apidl[0]);
        if (pItemId)
        {
            // Add basic menu items
            switch (static_cast<BigDriveItemType>(pItemId->uType))
            {
            case BigDriveItemType_Folder:
                // Add Open for folders
                ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, idCmdFirst + ID_OPEN, L"&Open");
                basicCmdCount++;

                // Add Explore for folders, if not in explorer mode
                if (!(uFlags & CMF_EXPLORE))
                {
                    ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, idCmdFirst + ID_OPEN, L"&Explore");
                    basicCmdCount++;
                }
                break;

            case BigDriveItemType_File:
                // Just "Open" for files
                ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, idCmdFirst + ID_OPEN, L"&Open");
                basicCmdCount++;
                break;
            }
        }
    }
    else if (m_cidl > 1)
    {
        // Multiple items selected - add generic actions
        ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, idCmdFirst + ID_OPEN, L"&Open");
        basicCmdCount++;
    }

    // Add separator after basic commands if we added any
    if (basicCmdCount > 0)
    {
        ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        basicCmdCount++;
    }

    // Add custom BigDrive commands
    UINT customCmdCount = AddBigDriveMenuItems(hmenu, indexMenu, idCmdFirst + ID_CUSTOM_COMMAND, idCmdLast);
    indexMenu += customCmdCount;

    // Add another separator if we added custom commands
    if (customCmdCount > 0)
    {
        ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        customCmdCount++;
    }

    // Try to get shell's default context menu
    UINT shellCmdCount = 0;
    if (!(uFlags & CMF_DEFAULTONLY))
    {
        IContextMenu* pShellCtxMenu = nullptr;
        HRESULT hr = GetShellContextMenu(&pShellCtxMenu);
        if (SUCCEEDED(hr) && pShellCtxMenu)
        {
            // Create a submenu for shell commands
            HMENU hSubmenu = ::CreatePopupMenu();
            if (hSubmenu)
            {
                // Get the standard shell menu items
                hr = pShellCtxMenu->QueryContextMenu(hSubmenu, 0,
                    idCmdFirst + ID_CUSTOM_COUNT,
                    idCmdLast,
                    CMF_NORMAL | CMF_EXPLORE | CMF_CANRENAME);

                if (SUCCEEDED(hr))
                {
                    // Store the shell menu for later use in InvokeCommand
                    if (m_pShellContextMenu)
                        m_pShellContextMenu->Release();

                    m_pShellContextMenu = pShellCtxMenu;
                    pShellCtxMenu->AddRef();

                    // Add the submenu to our menu
                    ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSubmenu, L"Shell Actions");
                    shellCmdCount = HRESULT_CODE(hr) + 1; // +1 for the submenu itself
                }
                else
                {
                    ::DestroyMenu(hSubmenu);
                }
            }

            pShellCtxMenu->Release();
        }
    }

    // Always add Properties at the end
    ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, idCmdFirst + ID_PROPERTIES, L"P&roperties");

    // Return the number of menu items added plus one for Properties
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, basicCmdCount + customCmdCount + shellCmdCount + 2);
}

/// <summary>
/// Carries out the command specified by the user from the context menu.
/// </summary>
IFACEMETHODIMP BigDriveShellContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    // Special case for keyboard shortcuts
    if (HIWORD(pici->lpVerb) != 0)
    {
        // Handle named verbs
        if (::lstrcmpiA(pici->lpVerb, "open") == 0)
        {
            // Handle Open verb
            // TODO: Implement the Open command for BigDrive items
            MessageBoxW(pici->hwnd, L"Open command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (::lstrcmpiA(pici->lpVerb, "properties") == 0)
        {
            // Handle Properties verb
            // TODO: Show properties dialog for BigDrive items
            MessageBoxW(pici->hwnd, L"Properties command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (::lstrcmpiA(pici->lpVerb, "refresh") == 0)
        {
            // Handle Refresh verb
            // TODO: Implement Refresh for BigDrive items
            MessageBoxW(pici->hwnd, L"Refresh command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (::lstrcmpiA(pici->lpVerb, "bigdrivecmd") == 0)
        {
            // Handle custom BigDrive command
            MessageBoxW(pici->hwnd, L"Custom BigDrive command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }

        // If we have a shell context menu, let it handle the verb
        if (m_pShellContextMenu)
        {
            return m_pShellContextMenu->InvokeCommand(pici);
        }

        return E_INVALIDARG;
    }
    else
    {
        // Handle by command ID
        UINT idCmd = LOWORD(pici->lpVerb);

        if (idCmd == ID_OPEN)
        {
            // Handle Open command
            // TODO: Implement the Open command for BigDrive items
            MessageBoxW(pici->hwnd, L"Open command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (idCmd == ID_PROPERTIES)
        {
            // Handle Properties command
            // TODO: Show properties dialog for BigDrive items
            MessageBoxW(pici->hwnd, L"Properties command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (idCmd == ID_REFRESH)
        {
            // Handle Refresh command
            // TODO: Implement Refresh for BigDrive items
            MessageBoxW(pici->hwnd, L"Refresh command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (idCmd == ID_CUSTOM_COMMAND)
        {
            // Handle custom BigDrive command
            MessageBoxW(pici->hwnd, L"Custom BigDrive command invoked", L"BigDrive", MB_OK);
            return S_OK;
        }
        else if (idCmd >= ID_CUSTOM_COUNT && m_pShellContextMenu)
        {
            // Adjust the command ID for the shell menu and let it handle the command
            CMINVOKECOMMANDINFO ici = *pici;
            ici.lpVerb = MAKEINTRESOURCEA(idCmd - ID_CUSTOM_COUNT);
            return m_pShellContextMenu->InvokeCommand(&ici);
        }
    }

    return E_INVALIDARG;
}

/// <summary>
/// Gets the help text for a context menu command.
/// </summary>
IFACEMETHODIMP BigDriveShellContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    // Handle requests for our own commands
    if (idCmd < ID_CUSTOM_COUNT)
    {
        switch (uType)
        {
        case GCS_HELPTEXTW:
            // Return help text for command in Unicode
            if (pszName && cchMax > 0)
            {
                LPWSTR pwszName = reinterpret_cast<LPWSTR>(pszName);

                switch (idCmd)
                {
                case ID_OPEN:
                    ::StringCchCopyW(pwszName, cchMax, L"Opens the selected item");
                    break;

                case ID_PROPERTIES:
                    ::StringCchCopyW(pwszName, cchMax, L"Displays properties of the selected item");
                    break;

                case ID_REFRESH:
                    ::StringCchCopyW(pwszName, cchMax, L"Refreshes the view");
                    break;

                case ID_CUSTOM_COMMAND:
                    ::StringCchCopyW(pwszName, cchMax, L"Performs a custom BigDrive operation");
                    break;

                default:
                    return E_INVALIDARG;
                }

                return S_OK;
            }
            return E_POINTER;

        case GCS_HELPTEXTA:
            // Return help text for command in ANSI
            if (pszName && cchMax > 0)
            {
                switch (idCmd)
                {
                case ID_OPEN:
                    ::StringCchCopyA(pszName, cchMax, "Opens the selected item");
                    break;

                case ID_PROPERTIES:
                    ::StringCchCopyA(pszName, cchMax, "Displays properties of the selected item");
                    break;

                case ID_REFRESH:
                    ::StringCchCopyA(pszName, cchMax, "Refreshes the view");
                    break;

                case ID_CUSTOM_COMMAND:
                    ::StringCchCopyA(pszName, cchMax, "Performs a custom BigDrive operation");
                    break;

                default:
                    return E_INVALIDARG;
                }

                return S_OK;
            }
            return E_POINTER;

        case GCS_VERBW:
            // Return verb for command in Unicode
            if (pszName && cchMax > 0 && idCmd < ARRAYSIZE(m_szVerbs))
            {
                ::StringCchCopyW(reinterpret_cast<LPWSTR>(pszName), cchMax, m_szVerbs[idCmd]);
                return S_OK;
            }
            return E_POINTER;

        case GCS_VERBA:
            // Return verb for command in ANSI
            if (pszName && cchMax > 0 && idCmd < ARRAYSIZE(m_szVerbs))
            {
                char szVerb[32];
                ::WideCharToMultiByte(CP_ACP, 0, m_szVerbs[idCmd], -1, szVerb, ARRAYSIZE(szVerb), NULL, NULL);
                ::StringCchCopyA(pszName, cchMax, szVerb);
                return S_OK;
            }
            return E_POINTER;
        }

        return E_NOTIMPL;
    }
    else if (m_pShellContextMenu)
    {
        // Pass the command to the shell menu
        return m_pShellContextMenu->GetCommandString(
            idCmd - ID_CUSTOM_COUNT, uType, pwReserved, pszName, cchMax);
    }

    return E_INVALIDARG;
}