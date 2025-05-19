// <copyright file="BigDriveShellFolderFactory.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include <unknwn.h> // For IClassFactory
#include "BigDriveShellFolder.h" // For BigDriveFolder



class BigDriveShellFolderFactory : public IClassFactory 
{

private:

    /// <summary>
    /// Static member that holds the absolute PIDL (Pointer to an Item ID List) representing the root location
    /// of the BigDrive namespace within the shell. This PIDL is used as a reference point for resolving
    /// relative item locations and for operations that require knowledge of the namespace root.
    /// </summary>
    static PIDLIST_ABSOLUTE s_pidlRoot;

    // Reference count for COM object
    LONG m_refCount; 

    /// <summary>
    /// The CLSID for the shell folder and the drive guid are the same.
    /// </summary>
    CLSID m_driveGuid;

public:

    /// <summary>
    /// Implements the COM IClassFactory interface to create instances of the BigDriveShellFolder class.
    /// This factory is responsible for producing shell folder objects associated with a specific drive GUID (CLSID),
    /// enabling integration of custom shell folders into Windows Explorer.
    /// It manages reference counting for COM lifetime management and supports standard IUnknown and IClassFactory methods,
    /// including QueryInterface, AddRef, Release, CreateInstance, and LockServer.
    /// Aggregation is not supported. The factory ensures that only supported interfaces are returned to clients.
    /// </summary>
    BigDriveShellFolderFactory(const CLSID& driveGuid) 
        : m_refCount(1), m_driveGuid(driveGuid) 
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IUnknown methods

    /// <summary>
    /// Implements the IUnknown::QueryInterface method for the class factory.
    /// Determines if the requested interface identifier (IID) is supported by the factory object.
    /// If the IID matches IID_IUnknown or IID_IClassFactory, returns a pointer to the IClassFactory interface and increments the reference count.
    /// Otherwise, sets the output pointer to nullptr and returns E_NOINTERFACE.
    /// This enables COM clients to query for supported interfaces on the class factory.
    /// </summary>
    /// <param name="riid">The interface identifier (IID) being requested.</param>
    /// <param name="ppvObject">Address of pointer variable that receives the interface pointer if successful.</param>
    /// <returns>S_OK if the interface is supported; otherwise, E_NOINTERFACE.</returns>
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override 
    {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) 
        {
            *ppvObject = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override 
    {
        return InterlockedIncrement(&m_refCount);
    }

    ULONG __stdcall Release() override {
        LONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) {
            delete this;
        }
        return ref;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // IClassFactory methods

    /// <summary>
    /// Implements the IClassFactory::CreateInstance method to instantiate BigDriveShellFolder objects.
    /// Creates a new shell folder object that represents the BigDrive namespace extension in Windows Explorer.
    /// The created object is initialized with the factory's drive GUID and is responsible for handling
    /// shell browsing operations like item enumeration, context menus, and property retrieval.
    /// 
    /// This method follows COM object creation semantics:
    /// - Returns E_INVALIDARG if ppvObject is NULL
    /// - Returns CLASS_E_NOAGGREGATION if aggregation is attempted (pUnkOuter != NULL)
    /// - Creates the shell folder object and queries it for the requested interface (riid)
    /// - Returns pointers only for supported interfaces
    /// </summary>
    /// <param name="pUnkOuter">Pointer to controlling IUnknown if aggregation is used. Must be NULL as aggregation is not supported.</param>
    /// <param name="riid">The interface identifier (IID) of the interface the caller wants to retrieve.</param>
    /// <param name="ppvObject">Address of pointer variable that receives the interface pointer if successful.</param>
    /// <returns>
    /// S_OK if successful; E_INVALIDARG if ppvObject is NULL;
    /// CLASS_E_NOAGGREGATION if pUnkOuter is not NULL; or other error codes if creation fails.
    /// </returns>
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;

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
