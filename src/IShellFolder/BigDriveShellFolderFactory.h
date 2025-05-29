// <copyright file="BigDriveShellFolderFactory.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include <unknwn.h> // For IClassFactory
#include "BigDriveShellFolder.h" // For BigDriveFolder

class BigDriveShellFolderFactory : public IClassFactory 
{

private:

    static BigDriveShellFolderEventLogger s_eventLogger;

private:

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
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

    ULONG __stdcall AddRef() override;

    ULONG __stdcall Release() override;

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

    /// <summary>
    /// Implements the IClassFactory::LockServer method to control the server's lifetime.
    /// This method is called by the COM runtime to increment or decrement the lock count
    /// on the class factory's server. When the lock count is nonzero, the server (typically
    /// a DLL) is kept loaded in memory, preventing it from being unloaded. This is useful
    /// for scenarios where clients may create and release objects frequently, and unloading
    /// the server would be inefficient.
    ///
    /// <para><b>Parameters:</b></para>
    /// <param name="fLock">
    ///   [in] If TRUE, increments the server's lock count to prevent unloading. If FALSE,
    ///   decrements the lock count, allowing the server to unload if the count reaches zero.
    /// </param>
    ///
    /// <para><b>Return Value:</b></para>
    /// <returns>
    ///   S_OK if the operation succeeds. Returns a COM error code if the operation fails.
    /// </returns>
    ///
    /// <para><b>Behavior and Notes:</b></para>
    /// <list type="bullet">
    ///   <item>This method is typically implemented by incrementing or decrementing a global
    ///         lock count (such as with InterlockedIncrement/InterlockedDecrement).</item>
    ///   <item>When the lock count is greater than zero, the COM runtime will not unload the
    ///         DLL, even if there are no active object references.</item>
    ///   <item>When the lock count reaches zero and there are no outstanding object references,
    ///         the server may be unloaded from memory.</item>
    ///   <item>This mechanism is important for in-process servers (DLLs) to manage their lifetime
    ///         efficiently and avoid unnecessary load/unload cycles.</item>
    ///   <item>For most shell extensions, a minimal implementation that always returns S_OK is
    ///         sufficient, unless you need to control DLL lifetime explicitly.</item>
    /// </list>
    ///
    /// <para><b>Typical Usage:</b></para>
    /// <list type="bullet">
    ///   <item>Called by COM when a client calls CoLockObjectExternal or similar APIs.</item>
    ///   <item>Used internally by COM to manage the lifetime of in-process servers.</item>
    /// </list>
    /// </summary>
    HRESULT __stdcall LockServer(BOOL fLock) override;
};
