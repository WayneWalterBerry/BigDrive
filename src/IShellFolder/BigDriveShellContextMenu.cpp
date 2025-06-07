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
/// Factory method to create an instance of BigDriveShellContextMenu.
/// </summary>
HRESULT BigDriveShellContextMenu::CreateInstance(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    
    *ppv = nullptr;
    
    BigDriveShellContextMenu* pContextMenu = new BigDriveShellContextMenu(pFolder, cidl, apidl);
    if (!pContextMenu)
    {
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr = pContextMenu->QueryInterface(IID_IContextMenu, ppv);
    if (FAILED(hr))
    {
        goto End;
    }


End:

    if (pContextMenu)
    {
        pContextMenu->Release();
        pContextMenu = nullptr;
    }
       
    return hr;
}

/// <summary>
/// Private constructor - use CreateInstance to create instances.
/// </summary>
BigDriveShellContextMenu::BigDriveShellContextMenu(BigDriveShellFolder* pFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl) :
    m_cRef(1),
    m_pFolder(pFolder),
    m_cidl(cidl),
    m_idCmdFirst(0),
    m_pShellContextMenu(nullptr)
{
    m_traceLogger.Initialize(pFolder->GetDriveGuid());

    // Keep a reference to the parent folder
    if (m_pFolder)
    {
        m_pFolder->AddRef();
    }

    m_traceLogger.Initialize(m_pFolder->GetDriveGuid());
    
    // Make a copy of the item IDs
    if (m_cidl > 0 && apidl)
    {
        m_apidl.reset(new PCITEMID_CHILD[m_cidl]);
        if (m_apidl)
        {
            for (UINT i = 0; i < m_cidl; i++)
            {
                m_apidl[i] = ILClone(apidl[i]);
            }
        }
        else
        {
            m_cidl = 0;
        }
    }
    
    // Initialize the verb strings
    ::StringCchCopyW(m_szVerbs[ID_OPEN], ARRAYSIZE(m_szVerbs[ID_OPEN]), L"open");
    ::StringCchCopyW(m_szVerbs[ID_PROPERTIES], ARRAYSIZE(m_szVerbs[ID_PROPERTIES]), L"properties");
    ::StringCchCopyW(m_szVerbs[ID_REFRESH], ARRAYSIZE(m_szVerbs[ID_REFRESH]), L"refresh");
    ::StringCchCopyW(m_szVerbs[ID_CUSTOM_COMMAND], ARRAYSIZE(m_szVerbs[ID_CUSTOM_COMMAND]), L"bigdrivecmd");
}

/// <summary>
/// Destructor.
/// </summary>
BigDriveShellContextMenu::~BigDriveShellContextMenu()
{
    // Free the item IDs
    if (m_apidl)
    {
        for (UINT i = 0; i < m_cidl; i++)
        {
            if (m_apidl[i])
            {
                ::ILFree((PITEMID_CHILD)m_apidl[i]);
            }
        }
    }

    m_traceLogger.Uninitialize();
    
    // Release the shell context menu if we have one
    if (m_pShellContextMenu)
    {
        m_pShellContextMenu->Release();
        m_pShellContextMenu = nullptr;
    }
    
    // Release the parent folder
    if (m_pFolder)
    {
        m_pFolder->Release();
        m_pFolder = nullptr;
    }
}

/// <summary>
/// Processes Windows messages sent to the context menu.
/// </summary>
IFACEMETHODIMP BigDriveShellContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Forward to HandleMenuMsg2 with NULL result pointer
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

/// <summary>
/// Processes Windows messages sent to the context menu and returns a result.
/// </summary>
IFACEMETHODIMP BigDriveShellContextMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
    // Initialize result to 0 if provided
    if (plResult)
        *plResult = 0;
    
    // Handle menu messages here if needed
    switch (uMsg)
    {
        case WM_MEASUREITEM:
        case WM_DRAWITEM:
        case WM_INITMENUPOPUP:
        case WM_MENUCHAR:
            // If we have a shell context menu that supports IContextMenu2/3, let it handle the messages
            if (m_pShellContextMenu)
            {
                IContextMenu3* pContextMenu3 = nullptr;
                HRESULT hr = m_pShellContextMenu->QueryInterface(IID_PPV_ARGS(&pContextMenu3));
                if (SUCCEEDED(hr))
                {
                    hr = pContextMenu3->HandleMenuMsg2(uMsg, wParam, lParam, plResult);
                    pContextMenu3->Release();
                    return hr;
                }
                
                IContextMenu2* pContextMenu2 = nullptr;
                hr = m_pShellContextMenu->QueryInterface(IID_PPV_ARGS(&pContextMenu2));
                if (SUCCEEDED(hr))
                {
                    hr = pContextMenu2->HandleMenuMsg(uMsg, wParam, lParam);
                    pContextMenu2->Release();
                    
                    // HandleMenuMsg doesn't set the result, so we have to do it for some messages
                    if (SUCCEEDED(hr) && plResult)
                    {
                        if (uMsg == WM_MENUCHAR)
                            *plResult = MAKELRESULT(0, MNC_CLOSE);
                    }
                    
                    return hr;
                }
            }
            break;
    }
    
    return S_OK;
}

/// <summary>
/// Gets the shell's default context menu handler for the selected items.
/// </summary>
HRESULT BigDriveShellContextMenu::GetShellContextMenu(IContextMenu** ppContextMenu)
{
    if (!ppContextMenu)
        return E_POINTER;

    *ppContextMenu = nullptr;

    if (!m_pFolder || !m_cidl || !m_apidl)
        return E_FAIL;

    // Get a data object for the selected items
    IDataObject* pDataObj = nullptr;
    HRESULT hr = m_pFolder->GetUIObjectOf(NULL, m_cidl, m_apidl.get(),
        IID_IDataObject, NULL, (void**)&pDataObj);

    if (SUCCEEDED(hr) && pDataObj)
    {
        // Use CDefFolderMenu_Create2 instead of SHCreateDefaultContextMenu
        // This function is more widely supported across Windows SDK versions
        hr = ::CDefFolderMenu_Create2(
            nullptr,                  // HWND (no owner window)
            nullptr,                  // No window handle needed
            m_cidl,                   // Number of items
            m_apidl.get(),            // Array of item IDs
            m_pFolder,                // Parent shell folder
            nullptr,                  // No callback
            0,                        // No verb count
            nullptr,                  // No verbs
            ppContextMenu             // Output context menu
        );

        // Release the data object since we're not using it
        pDataObj->Release();
    }

    return hr;
}

/// <summary>
/// Initializes the context menu with BigDrive-specific commands.
/// </summary>
UINT BigDriveShellContextMenu::AddBigDriveMenuItems(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast)
{
    // Add a Refresh command
    ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, m_idCmdFirst + ID_REFRESH, L"&Refresh");
    
    // Add a custom BigDrive command
    ::InsertMenuW(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING, m_idCmdFirst + ID_CUSTOM_COMMAND, L"BigDrive &Command");
    
    // Return number of items added
    return 2;
}