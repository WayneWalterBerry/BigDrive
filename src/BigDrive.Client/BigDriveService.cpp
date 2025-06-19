// <copyright file="BigDriveService.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveService.h"
#include "Interfaces\IBigDriveProvision.h"
#include "GuidUtil.h"

#include <windows.h>
#include <guiddef.h>
#include <wchar.h>
#include <initguid.h>
#include <shlobj.h>

DEFINE_GUID(CLSID_BigDriveService, 0xE6F5A1B2, 0x4C6E, 0x4F8A, 0x9D, 0x3E, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x7F);
DEFINE_GUID(IID_IBigDriveProvision, 0x293d4995, 0xfdfb, 0x46fd, 0xa0, 0xc6, 0xa7, 0xde, 0x2d, 0xa5, 0xb1, 0x3f);

using namespace BigDriveClient;

/// <summary>
/// Calls the BigDrive.Service IBigDriveProvision interface to create a drive for the Sample Provider.
/// </summary>
/// <returns>HRESULT indicating success or failure.</returns>
HRESULT BigDriveService::CreateSampleProviderDrive()
{
    HRESULT hr = S_OK;
    IBigDriveProvision* pProvision = nullptr;
    BSTR bstrJsonConfig = nullptr;
    LPITEMIDLIST pidlMyComputer = nullptr;

    // Predefined GUID for the Sample Provider drive
    GUID driveGuid = { 0x0F7C17BF, 0x4CD6, 0x4D54, { 0x86, 0x42, 0x1C, 0x55, 0xE7, 0x9B, 0x44, 0x21 } };

    // CLSID for the Sample Provider (from your test code)
    GUID clsidSampleProvider = { 0xF8FE2E5A, 0xE8B8, 0x4207, { 0xBC, 0x04, 0xEA, 0x4B, 0xCD, 0x4C, 0x43, 0x61 } };

    // Compose the JSON configuration string
    // Example: {"id":"...","name":"Sample Drive","clsid":"..."}
    wchar_t wszDriveGuid[64] = { 0 };
    wchar_t wszClsid[64] = { 0 };
    StringFromGUID(driveGuid, wszDriveGuid, 64);
    StringFromGUID(clsidSampleProvider, wszClsid, 64);

    wchar_t wszJson[512] = { 0 };
    ::swprintf(wszJson, 512,
        L"{\"id\":\"%s\",\"name\":\"Sample Drive\",\"clsid\":\"%s\"}",
        wszDriveGuid, wszClsid);

    bstrJsonConfig = ::SysAllocString(wszJson);
    if (bstrJsonConfig == nullptr)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }

    // Create the BigDriveProvision COM object
    hr = ::CoCreateInstance(
        CLSID_BigDriveService,
        nullptr,
        CLSCTX_LOCAL_SERVER,
        IID_IBigDriveProvision,
        (void**)&pProvision);
    if (FAILED(hr))
    {
        goto End;
    }

    // Call CreateFromConfiguration
    hr = pProvision->CreateFromConfiguration(bstrJsonConfig);
    if (FAILED(hr))
    {
        goto End;
    }

    hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
    if (SUCCEEDED(hr) && pidlMyComputer != nullptr)
    {
        ::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlMyComputer, NULL);
    }

End:

    if (pidlMyComputer)
    {
        ::CoTaskMemFree(pidlMyComputer);
        pidlMyComputer = nullptr;
    }

    if (bstrJsonConfig)
    {
        ::SysFreeString(bstrJsonConfig);
        bstrJsonConfig = nullptr;
    }

    if (pProvision)
    {
        pProvision->Release();
        pProvision = nullptr;
    }

    return hr;
}

