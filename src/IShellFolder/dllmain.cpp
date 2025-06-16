// <copyright file="dllmain.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

// Header
#include "dllmain.h"

// System
#include <windows.h>
#include <combaseapi.h>

/// Local
#include "BigDriveShellFolderFactory.h"
#include "LaunchDebugger.h"
#include "RegistrationManager.h"
#include "Logging\BigDriveTraceLogger.h"
#include "..\BigDrive.Client\ApplicationManager.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    HRESULT hr = S_OK;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        BigDriveTraceLogger::Initialize();

        // Initialize COM
        hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr)) 
        {
            // Handle the error (e.g., log it or return FALSE to indicate failure)
            return FALSE;
        }


        break;
    case DLL_PROCESS_DETACH:
        
        BigDriveTraceLogger::Uninitialize();

        // Uninitialize COM
        ::CoUninitialize();
        break;
    }

    return TRUE;
}

/// <summary>
/// Registers the COM server and its components with the system.
/// This function performs registration of all COM+ applications (Providers) and their components that support the
/// IBigDriveRegistration interface,and registers all BigDrive shell folders by creating the necessary registry entries 
/// for Windows Explorer integration.
/// Returns S_OK if registration succeeds, or an error HRESULT if any step fails.
/// </summary>
extern "C" HRESULT __stdcall DllRegisterServer()
{
    HRESULT hr = S_OK;
    bool bitMatch = FALSE;

    hr = RegistrationManager::CheckDllAndOSBitnessMatch(bitMatch);
    if (FAILED(hr))
    {
        // Log the error and return failure.
        goto End;
    }

    if (!bitMatch)
    {
        // Log a message indicating that the bitness of the DLL and OS do not match.
        hr = E_FAIL;
        goto End;
    }

	hr = WriteShellDllLocationToRegistry();
    if (FAILED(hr))
    {
        // Log the error and return failure.
        goto End;
    }

    /*
    // Registers all COM+ applications (providers) and their components that support the IBigDriveRegistration interface.
    // This method enumerates applications and their components using the COMAdminCatalog, queries for the
    // IBigDriveRegistration interface, and invokes the Register method on each supported component.
    hr = ApplicationManager::RegisterApplications();
    if (FAILED(hr))
    {
        goto End;
    }

    // Scans all CLSID entries in the Windows registry and removes those associated with BigDrive shell folders.
    // This method identifies shell folders registered by BigDrive by checking the InprocServer32 path for the
    // "BigDrive.ShellFolder" substring, then unregisters and deletes their related registry keys.
    RegistrationManager::CleanUpShellFolders();

    // Refresh the Windows Explorer shell to reflect the changes made by the cleanup process.
    ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, NULL, NULL);

    // Refresh the desktop to ensure that any changes made to the desktop folder are reflected immediately.
    ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, L"C:\\Users\\Public\\Desktop", NULL);



    /// Enumerates all registered drive GUIDs from the registry, retrieves their configuration,
    /// and registers each as a shell folder in Windows Explorer. For each drive, this method
    /// obtains its configuration, then creates the necessary registry entries to expose the
    /// drive as an IShellFolder. Logs errors and informational messages for each operation.
    hr = RegistrationManager::RegisterShellFoldersFromRegistry();
    if (FAILED(hr))
    {
        goto End;
    }

    // Refresh the Windows Explorer shell to reflect the changes made by the registration process.
    ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, NULL, NULL);

    // Refresh the desktop to ensure that any changes made to the desktop folder are reflected immediately.
    ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, L"C:\\Users\\Public\\Desktop", NULL);
    */

End:
  
    return hr;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    return S_OK;
}

/// <summary>
/// Retrieves a class object from a DLL for the specified CLSID.
/// This function is typically implemented in a DLL that provides COM objects,
/// allowing clients to obtain an IShellFolder instance.
/// 
/// Usage:
/// Clients call DllGetClassObject() to retrieve an IClassFactory for IShellFolder,
/// which is then used to instantiate the requested shell folder object.
/// </summary>
/// <param name="rclsid">The CLSID of the object to retrieve.</param>
/// <param name="riid">The interface identifier (IID) for the requested interface.</param>
/// <param name="ppv">Pointer to the location where the interface pointer will be stored.</param>
/// <returns>HRESULT indicating success or failure.</returns>
extern "C" HRESULT __stdcall DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
    HRESULT hr = S_OK;
    CLSID* pclsid = nullptr;
    DWORD dwSize = 0;
    BigDriveShellFolderFactory* pFactory = nullptr;

    BigDriveTraceLogger::LogEnter(__FUNCTION__, rclsid, riid);

    if (ppv == nullptr)
    {
        hr = E_POINTER;
        goto End;
    }

    // Ensure the output pointer is initialized to nullptr.
    *ppv = nullptr;

    hr = RegistrationManager::GetRegisteredCLSIDs(&pclsid, dwSize);
    if (FAILED(hr))
    {
        goto End;
    }

    for (int i = 0; pclsid[i] != GUID_NULL; i++)
    {
        if (pclsid[i] == rclsid)
        {
            // The CLSID matches, create the factory, with the CLSID as the drive guid
            // the Shell Folder is registered as a COM component using the drive guid.
            pFactory = new BigDriveShellFolderFactory(rclsid);
            if (!pFactory)
            {
                hr = E_OUTOFMEMORY;
                goto End;
            }

            hr = pFactory->QueryInterface(riid, ppv);
            if (FAILED(hr))
            {
                goto End;
            }

            goto End;
        }
    }

    hr = CLASS_E_CLASSNOTAVAILABLE;

End:

    // Clean Up

    if (pFactory)
    {
        pFactory->Release();
        pFactory = nullptr;
    }

    if (pclsid)
    {
        CoTaskMemFree(pclsid);
        pclsid = nullptr;
    }

    BigDriveTraceLogger::LogExit(__FUNCTION__, hr);

    return hr;
}


/// <inheritdoc/>
HRESULT WriteShellDllLocationToRegistry()
{
    HMODULE hModule = nullptr;
    WCHAR dllPath[MAX_PATH] = { 0 };
    HKEY hKey = nullptr;
    HRESULT hr = S_OK;
    LONG lResult = 0;

    // Registry path: HKLM\SOFTWARE\BigDrive\ShellFolder
    LPCWSTR subKey = L"SOFTWARE\\BigDrive\\ShellFolder";
    LPCWSTR valueName = L"ShellDll";

    // Get the module handle for the current DLL
    if (!::GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&WriteShellDllLocationToRegistry),
        &hModule))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto End;
    }

    // Get the full path of the DLL
    if (::GetModuleFileNameW(hModule, dllPath, MAX_PATH) == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto End;
    }

    lResult = ::RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        subKey,
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        nullptr,
        &hKey,
        nullptr);

    if (lResult != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto End;
    }

    lResult = ::RegSetValueExW(
        hKey,
        valueName,
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(dllPath),
        static_cast<DWORD>((::lstrlenW(dllPath) + 1) * sizeof(WCHAR)));

    if (lResult != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lResult);
        goto End;
    }

End:

    if (hKey != nullptr)
    {
        ::RegCloseKey(hKey);
        hKey = nullptr;
    }

    return hr;

}

