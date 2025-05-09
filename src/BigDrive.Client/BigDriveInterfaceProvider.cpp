// <copyright file="BigDriveInterfaceProvider.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include <comdef.h>
#include <iostream>

#include "BigDriveInterfaceProvider.h"

HRESULT BigDriveInterfaceProvider::GetSupportedInterfaceIDs(const CLSID& clsid, std::vector<IID>& interfaceIDs)
{
    HRESULT hr = S_OK;
    IUnknown* pUnknown = nullptr;

    // Create an instance of the COM class
    hr = CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown, reinterpret_cast<void**>(&pUnknown));
    if (FAILED(hr))
    {
        std::wcerr << L"Failed to create COM instance. HRESULT: " << hr << std::endl;
        return hr;
    }

    // Query for supported interfaces
    ITypeInfo* pTypeInfo = nullptr;
    ITypeLib* pTypeLib = nullptr;
    TYPEATTR* pTypeAttr = nullptr;

    // Get the ITypeInfo for the class
    hr = pUnknown->QueryInterface(IID_IProvideClassInfo, reinterpret_cast<void**>(&pTypeInfo));
    if (SUCCEEDED(hr))
    {
        hr = pTypeInfo->GetContainingTypeLib(&pTypeLib, nullptr);
        if (SUCCEEDED(hr))
        {
            hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
            if (SUCCEEDED(hr))
            {
                // Iterate through the implemented interfaces
                for (UINT i = 0; i < pTypeAttr->cImplTypes; ++i)
                {
                    HREFTYPE hRefType;
                    hr = pTypeInfo->GetRefTypeOfImplType(i, &hRefType);
                    if (SUCCEEDED(hr))
                    {
                        ITypeInfo* pRefTypeInfo = nullptr;
                        hr = pTypeInfo->GetRefTypeInfo(hRefType, &pRefTypeInfo);
                        if (SUCCEEDED(hr))
                        {
                            TYPEATTR* pRefTypeAttr = nullptr;
                            hr = pRefTypeInfo->GetTypeAttr(&pRefTypeAttr);
                            if (SUCCEEDED(hr))
                            {
                                // Add the IID to the list
                                interfaceIDs.push_back(pRefTypeAttr->guid);
                                pRefTypeInfo->ReleaseTypeAttr(pRefTypeAttr);
                            }
                            pRefTypeInfo->Release();
                        }
                    }
                }
                pTypeInfo->ReleaseTypeAttr(pTypeAttr);
            }
            pTypeLib->Release();
        }
        pTypeInfo->Release();
    }

    // Release the COM object
    pUnknown->Release();

    return hr;
}
