// <copyright file="dllmain.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"
#include <unknwn.h> // For IClassFactory
#include "BigDriveShellFolder.h" // For BigDriveFolder

class BigDriveShellFolderFactory : public IClassFactory {
private:
    LONG m_refCount; // Reference count for COM object

public:
    BigDriveShellFolderFactory() : m_refCount(1) {}

    // IUnknown methods
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppvObject = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    ULONG __stdcall Release() override {
        LONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) {
            delete this;
        }
        return ref;
    }

    // IClassFactory methods
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override {
        if (pUnkOuter != nullptr) {
            // Aggregation is not supported
            return CLASS_E_NOAGGREGATION;
        }

        // Create an instance of BigDriveFolder
        BigDriveFolder* pFolder = new (std::nothrow) BigDriveFolder();
        if (!pFolder) {
            return E_OUTOFMEMORY;
        }

        // Query the requested interface
        HRESULT hr = pFolder->QueryInterface(riid, ppvObject);
        pFolder->Release(); // Release the initial reference held by the factory
        return hr;
    }

    HRESULT __stdcall LockServer(BOOL fLock) override {
        // Lock or unlock the server
        if (fLock) {
            InterlockedIncrement(&m_refCount);
        } else {
            InterlockedDecrement(&m_refCount);
        }
        return S_OK;
    }
};
