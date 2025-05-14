// ICOMAdminCatalog2.h - C-style declarations for COM+ Admin Catalog

#pragma once

#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <oaidl.h>

// Forward declarations for types used in ICOMAdminCatalog2
typedef enum COMAdminInUse {
    COMAdminNotInUse = 0,
    COMAdminInUseByCatalog = 0x1,
    COMAdminInUseByRegistryUnknown = 0x2,
    COMAdminInUseByRegistryProxyStub = 0x3,
    COMAdminInUseByRegistryTypeLib = 0x4,
    COMAdminInUseByRegistryClsid = 0x5
} COMAdminInUse;

#ifdef __cplusplus
extern "C" {
#endif

    // Define CINTERFACE to ensure C-style declarations
#ifndef CINTERFACE
#define CINTERFACE
#endif

// Forward declarations
    typedef interface ICOMAdminCatalog ICOMAdminCatalog;
    typedef interface ICOMAdminCatalog2 ICOMAdminCatalog2;

    // GUID for ICOMAdminCatalog2 interface
    DEFINE_GUID(IID_ICOMAdminCatalog2,
        0x790C6E0B, 0x9194, 0x4cc9, 0x94, 0x26, 0xA4, 0x8A, 0x63, 0x18, 0x56, 0x96);

    // ICOMAdminCatalog2 interface Vtable
    typedef struct ICOMAdminCatalog2Vtbl
    {
        // IUnknown methods
        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            ICOMAdminCatalog2* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_ void** ppvObject);

        ULONG(STDMETHODCALLTYPE* AddRef)(
            ICOMAdminCatalog2* This);

        ULONG(STDMETHODCALLTYPE* Release)(
            ICOMAdminCatalog2* This);

        // IDispatch methods
        HRESULT(STDMETHODCALLTYPE* GetTypeInfoCount)(
            ICOMAdminCatalog2* This,
            /* [out] */ UINT* pctinfo);

        HRESULT(STDMETHODCALLTYPE* GetTypeInfo)(
            ICOMAdminCatalog2* This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo** ppTInfo);

        HRESULT(STDMETHODCALLTYPE* GetIDsOfNames)(
            ICOMAdminCatalog2* This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR* rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID* rgDispId);

        /* [local] */ HRESULT(STDMETHODCALLTYPE* Invoke)(
            ICOMAdminCatalog2* This,
            /* [annotation][in] */
            _In_ DISPID dispIdMember,
            /* [annotation][in] */
            _In_ REFIID riid,
            /* [annotation][in] */
            _In_ LCID lcid,
            /* [annotation][in] */
            _In_ WORD wFlags,
            /* [annotation][out][in] */
            _In_ DISPPARAMS* pDispParams,
            /* [annotation][out] */
            _Out_opt_ VARIANT* pVarResult,
            /* [annotation][out] */
            _Out_opt_ EXCEPINFO* pExcepInfo,
            /* [annotation][out] */
            _Out_opt_ UINT* puArgErr);

        // ICOMAdminCatalog methods
        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetCollection)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrCollName,
            /* [retval][out] */ IDispatch** ppCatalogCollection);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* Connect)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrCatalogServerName,
            /* [retval][out] */ IDispatch** ppCatalogCollection);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_MajorVersion)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ long* plMajorVersion);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_MinorVersion)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ long* plMinorVersion);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetCollectionByQuery)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrCollName,
            /* [in] */ SAFEARRAY** ppsaVarQuery,
            /* [retval][out] */ IDispatch** ppCatalogCollection);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ImportComponent)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIDOrName,
            /* [in] */ BSTR bstrCLSIDOrProgID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* InstallComponent)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIDOrName,
            /* [in] */ BSTR bstrDLL,
            /* [in] */ BSTR bstrTLB,
            /* [in] */ BSTR bstrPSDLL);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ShutdownApplication)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIDOrName);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ExportApplication)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIDOrName,
            /* [in] */ BSTR bstrApplicationFile,
            /* [in] */ long lOptions);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* InstallApplication)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationFile,
            /* [optional][in] */ BSTR bstrDestinationDirectory,
            /* [optional][in] */ long lOptions,
            /* [optional][in] */ BSTR bstrUserId,
            /* [optional][in] */ BSTR bstrPassword,
            /* [optional][in] */ BSTR bstrRSN);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* StopRouter)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* RefreshRouter)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* StartRouter)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* Reserved1)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* Reserved2)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* InstallMultipleComponents)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIDOrName,
            /* [in] */ SAFEARRAY** ppsaVarFileNames,
            /* [in] */ SAFEARRAY** ppsaVarCLSIDs);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetMultipleComponentsInfo)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIdOrName,
            /* [in] */ SAFEARRAY** ppsaVarFileNames,
            /* [out] */ SAFEARRAY** ppsaVarCLSIDs,
            /* [out] */ SAFEARRAY** ppsaVarClassNames,
            /* [out] */ SAFEARRAY** ppsaVarFileFlags,
            /* [out] */ SAFEARRAY** ppsaVarComponentFlags);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* RefreshComponents)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* BackupREGDB)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrBackupFilePath);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* RestoreREGDB)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrBackupFilePath);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* QueryApplicationFile)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationFile,
            /* [out] */ BSTR* pbstrApplicationName,
            /* [out] */ BSTR* pbstrApplicationDescription,
            /* [out] */ VARIANT_BOOL* pbHasUsers,
            /* [out] */ VARIANT_BOOL* pbIsProxy,
            /* [out] */ SAFEARRAY** ppsaVarFileNames);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* StartApplication)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIdOrName);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ServiceCheck)(
            ICOMAdminCatalog2* This,
            /* [in] */ long lService,
            /* [retval][out] */ long* plStatus);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* InstallMultipleEventClasses)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIdOrName,
            /* [in] */ SAFEARRAY** ppsaVarFileNames,
            /* [in] */ SAFEARRAY** ppsaVarCLSIDS);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* InstallEventClass)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplIdOrName,
            /* [in] */ BSTR bstrDLL,
            /* [in] */ BSTR bstrTLB,
            /* [in] */ BSTR bstrPSDLL);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetEventClassesForIID)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrIID,
            /* [out] */ SAFEARRAY** ppsaVarCLSIDs,
            /* [out] */ SAFEARRAY** ppsaVarProgIDs,
            /* [out] */ SAFEARRAY** ppsaVarDescriptions);

        // ICOMAdminCatalog2 specific methods
        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetCollectionByQuery2)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrCollectionName,
            /* [in] */ VARIANT* pVarQueryStrings,
            /* [retval][out] */ IDispatch** ppCatalogCollection);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetApplicationInstanceIDFromProcessID)(
            ICOMAdminCatalog2* This,
            /* [in] */ long lProcessID,
            /* [retval][out] */ BSTR* pbstrApplicationInstanceID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ShutdownApplicationInstances)(
            ICOMAdminCatalog2* This,
            /* [in] */ VARIANT* pVarApplicationInstanceID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* PauseApplicationInstances)(
            ICOMAdminCatalog2* This,
            /* [in] */ VARIANT* pVarApplicationInstanceID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ResumeApplicationInstances)(
            ICOMAdminCatalog2* This,
            /* [in] */ VARIANT* pVarApplicationInstanceID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* RecycleApplicationInstances)(
            ICOMAdminCatalog2* This,
            /* [in] */ VARIANT* pVarApplicationInstanceID,
            /* [in] */ long lReasonCode);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* AreApplicationInstancesPaused)(
            ICOMAdminCatalog2* This,
            /* [in] */ VARIANT* pVarApplicationInstanceID,
            /* [retval][out] */ VARIANT_BOOL* pVarBoolPaused);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* DumpApplicationInstance)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationInstanceID,
            /* [in] */ BSTR bstrDirectory,
            /* [in] */ long lMaxImages,
            /* [retval][out] */ BSTR* pbstrDumpFile);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_IsApplicationInstanceDumpSupported)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ VARIANT_BOOL* pVarBoolDumpSupported);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* CreateServiceForApplication)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName,
            /* [in] */ BSTR bstrServiceName,
            /* [in] */ BSTR bstrStartType,
            /* [in] */ BSTR bstrErrorControl,
            /* [in] */ BSTR bstrDependencies,
            /* [in] */ BSTR bstrRunAs,
            /* [in] */ BSTR bstrPassword,
            /* [in] */ VARIANT_BOOL bDesktopOk);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* DeleteServiceForApplication)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetPartitionID)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName,
            /* [retval][out] */ BSTR* pbstrPartitionID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetPartitionName)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName,
            /* [retval][out] */ BSTR* pbstrPartitionName);

        /* [helpstring][id][propput] */ HRESULT(STDMETHODCALLTYPE* put_CurrentPartition)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrPartitionIDOrName);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_CurrentPartitionID)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ BSTR* pbstrPartitionID);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_CurrentPartitionName)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ BSTR* pbstrPartitionName);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_GlobalPartitionID)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ BSTR* pbstrGlobalPartitionID);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* FlushPartitionCache)(
            ICOMAdminCatalog2* This);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* CopyApplications)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrSourcePartitionIDOrName,
            /* [in] */ VARIANT* pVarApplicationID,
            /* [in] */ BSTR bstrDestinationPartitionIDOrName);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* CopyComponents)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrSourceApplicationIDOrName,
            /* [in] */ VARIANT* pVarCLSIDOrProgID,
            /* [in] */ BSTR bstrDestinationApplicationIDOrName);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* MoveComponents)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrSourceApplicationIDOrName,
            /* [in] */ VARIANT* pVarCLSIDOrProgID,
            /* [in] */ BSTR bstrDestinationApplicationIDOrName);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* AliasComponent)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrSrcApplicationIDOrName,
            /* [in] */ BSTR bstrCLSIDOrProgID,
            /* [in] */ BSTR bstrDestApplicationIDOrName,
            /* [in] */ BSTR bstrNewProgId,
            /* [in] */ BSTR bstrNewClsid);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* IsSafeToDelete)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrDllName,
            /* [retval][out] */ COMAdminInUse* pCOMAdminInUse);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ImportUnconfiguredComponents)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName,
            /* [in] */ VARIANT* pVarCLSIDOrProgID,
            /* [optional][in] */ VARIANT* pVarComponentType);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* PromoteUnconfiguredComponents)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName,
            /* [in] */ VARIANT* pVarCLSIDOrProgID,
            /* [optional][in] */ VARIANT* pVarComponentType);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ImportComponents)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationIDOrName,
            /* [in] */ VARIANT* pVarCLSIDOrProgID,
            /* [optional][in] */ VARIANT* pVarComponentType);

        /* [helpstring][id][propget] */ HRESULT(STDMETHODCALLTYPE* get_Is64BitCatalogServer)(
            ICOMAdminCatalog2* This,
            /* [retval][out] */ VARIANT_BOOL* pbIs64Bit);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* ExportPartition)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrPartitionIDOrName,
            /* [in] */ BSTR bstrPartitionFileName,
            /* [in] */ long lOptions);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* InstallPartition)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ BSTR bstrDestDirectory,
            /* [in] */ long lOptions,
            /* [in] */ BSTR bstrUserID,
            /* [in] */ BSTR bstrPassword,
            /* [in] */ BSTR bstrRSN);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* QueryApplicationFile2)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrApplicationFile,
            /* [retval][out] */ IDispatch** ppFilesForImport);

        /* [helpstring][id] */ HRESULT(STDMETHODCALLTYPE* GetComponentVersionCount)(
            ICOMAdminCatalog2* This,
            /* [in] */ BSTR bstrCLSIDOrProgID,
            /* [retval][out] */ long* plVersionCount);

    } ICOMAdminCatalog2Vtbl;

    // Interface definition
    typedef struct ICOMAdminCatalog2 {
        CONST_VTBL struct ICOMAdminCatalog2Vtbl* lpVtbl;
    } ICOMAdminCatalog2;

    // Macros for calling interface methods through the vtable
#ifdef COBJMACROS

// IUnknown methods
#define ICOMAdminCatalog2_QueryInterface(This,riid,ppvObject) \
    ( (This)->lpVtbl->QueryInterface(This,riid,ppvObject) )
#define ICOMAdminCatalog2_AddRef(This) \
    ( (This)->lpVtbl->AddRef(This) )
#define ICOMAdminCatalog2_Release(This) \
    ( (This)->lpVtbl->Release(This) )

// IDispatch methods
#define ICOMAdminCatalog2_GetTypeInfoCount(This,pctinfo) \
    ( (This)->lpVtbl->GetTypeInfoCount(This,pctinfo) )
#define ICOMAdminCatalog2_GetTypeInfo(This,iTInfo,lcid,ppTInfo) \
    ( (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo) )
#define ICOMAdminCatalog2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) \
    ( (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) )
#define ICOMAdminCatalog2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) \
    ( (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) )

// ICOMAdminCatalog methods
#define ICOMAdminCatalog2_GetCollection(This,bstrCollName,ppCatalogCollection) \
    ( (This)->lpVtbl->GetCollection(This,bstrCollName,ppCatalogCollection) )
#define ICOMAdminCatalog2_Connect(This,bstrCatalogServerName,ppCatalogCollection) \
    ( (This)->lpVtbl->Connect(This,bstrCatalogServerName,ppCatalogCollection) )
#define ICOMAdminCatalog2_get_MajorVersion(This,plMajorVersion) \
    ( (This)->lpVtbl->get_MajorVersion(This,plMajorVersion) )
#define ICOMAdminCatalog2_get_MinorVersion(This,plMinorVersion) \
    ( (This)->lpVtbl->get_MinorVersion(This,plMinorVersion) )
#define ICOMAdminCatalog2_GetCollectionByQuery(This,bstrCollName,ppsaVarQuery,ppCatalogCollection) \
    ( (This)->lpVtbl->GetCollectionByQuery(This,bstrCollName,ppsaVarQuery,ppCatalogCollection) )
#define ICOMAdminCatalog2_ImportComponent(This,bstrApplIDOrName,bstrCLSIDOrProgID) \
    ( (This)->lpVtbl->ImportComponent(This,bstrApplIDOrName,bstrCLSIDOrProgID) )
#define ICOMAdminCatalog2_InstallComponent(This,bstrApplIDOrName,bstrDLL,bstrTLB,bstrPSDLL) \
    ( (This)->lpVtbl->InstallComponent(This,bstrApplIDOrName,bstrDLL,bstrTLB,bstrPSDLL) )
#define ICOMAdminCatalog2_ShutdownApplication(This,bstrApplIDOrName) \
    ( (This)->lpVtbl->ShutdownApplication(This,bstrApplIDOrName) )
#define ICOMAdminCatalog2_ExportApplication(This,bstrApplIDOrName,bstrApplicationFile,lOptions) \
    ( (This)->lpVtbl->ExportApplication(This,bstrApplIDOrName,bstrApplicationFile,lOptions) )
#define ICOMAdminCatalog2_InstallApplication(This,bstrApplicationFile,bstrDestinationDirectory,lOptions,bstrUserId,bstrPassword,bstrRSN) \
    ( (This)->lpVtbl->InstallApplication(This,bstrApplicationFile,bstrDestinationDirectory,lOptions,bstrUserId,bstrPassword,bstrRSN) )
#define ICOMAdminCatalog2_StopRouter(This) \
    ( (This)->lpVtbl->StopRouter(This) )
#define ICOMAdminCatalog2_RefreshRouter(This) \
    ( (This)->lpVtbl->RefreshRouter(This) )
#define ICOMAdminCatalog2_StartRouter(This) \
    ( (This)->lpVtbl->StartRouter(This) )
#define ICOMAdminCatalog2_Reserved1(This) \
    ( (This)->lpVtbl->Reserved1(This) )
#define ICOMAdminCatalog2_Reserved2(This) \
    ( (This)->lpVtbl->Reserved2(This) )
#define ICOMAdminCatalog2_InstallMultipleComponents(This,bstrApplIDOrName,ppsaVarFileNames,ppsaVarCLSIDs) \
    ( (This)->lpVtbl->InstallMultipleComponents(This,bstrApplIDOrName,ppsaVarFileNames,ppsaVarCLSIDs) )
#define ICOMAdminCatalog2_GetMultipleComponentsInfo(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDs,ppsaVarClassNames,ppsaVarFileFlags,ppsaVarComponentFlags) \
    ( (This)->lpVtbl->GetMultipleComponentsInfo(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDs,ppsaVarClassNames,ppsaVarFileFlags,ppsaVarComponentFlags) )
#define ICOMAdminCatalog2_RefreshComponents(This) \
    ( (This)->lpVtbl->RefreshComponents(This) )
#define ICOMAdminCatalog2_BackupREGDB(This,bstrBackupFilePath) \
    ( (This)->lpVtbl->BackupREGDB(This,bstrBackupFilePath) )
#define ICOMAdminCatalog2_RestoreREGDB(This,bstrBackupFilePath) \
    ( (This)->lpVtbl->RestoreREGDB(This,bstrBackupFilePath) )
#define ICOMAdminCatalog2_QueryApplicationFile(This,bstrApplicationFile,pbstrApplicationName,pbstrApplicationDescription,pbHasUsers,pbIsProxy,ppsaVarFileNames) \
    ( (This)->lpVtbl->QueryApplicationFile(This,bstrApplicationFile,pbstrApplicationName,pbstrApplicationDescription,pbHasUsers,pbIsProxy,ppsaVarFileNames) )
#define ICOMAdminCatalog2_StartApplication(This,bstrApplIdOrName) \
    ( (This)->lpVtbl->StartApplication(This,bstrApplIdOrName) )
#define ICOMAdminCatalog2_ServiceCheck(This,lService,plStatus) \
    ( (This)->lpVtbl->ServiceCheck(This,lService,plStatus) )
#define ICOMAdminCatalog2_InstallMultipleEventClasses(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDS) \
    ( (This)->lpVtbl->InstallMultipleEventClasses(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDS) )
#define ICOMAdminCatalog2_InstallEventClass(This,bstrApplIdOrName,bstrDLL,bstrTLB,bstrPSDLL) \
    ( (This)->lpVtbl->InstallEventClass(This,bstrApplIdOrName,bstrDLL,bstrTLB,bstrPSDLL) )
#define ICOMAdminCatalog2_GetEventClassesForIID(This,bstrIID,ppsaVarCLSIDs,ppsaVarProgIDs,ppsaVarDescriptions) \
    ( (This)->lpVtbl->GetEventClassesForIID(This,bstrIID,ppsaVarCLSIDs,ppsaVarProgIDs,ppsaVarDescriptions) )

// ICOMAdminCatalog2 methods
#define ICOMAdminCatalog2_GetCollectionByQuery2(This,bstrCollectionName,pVarQueryStrings,ppCatalogCollection) \
    ( (This)->lpVtbl->GetCollectionByQuery2(This,bstrCollectionName,pVarQueryStrings,ppCatalogCollection) )
#define ICOMAdminCatalog2_GetApplicationInstanceIDFromProcessID(This,lProcessID,pbstrApplicationInstanceID) \
    ( (This)->lpVtbl->GetApplicationInstanceIDFromProcessID(This,lProcessID,pbstrApplicationInstanceID) )
#define ICOMAdminCatalog2_ShutdownApplicationInstances(This,pVarApplicationInstanceID) \
    ( (This)->lpVtbl->ShutdownApplicationInstances(This,pVarApplicationInstanceID) )
#define ICOMAdminCatalog2_PauseApplicationInstances(This,pVarApplicationInstanceID) \
    ( (This)->lpVtbl->PauseApplicationInstances(This,pVarApplicationInstanceID) )
#define ICOMAdminCatalog2_ResumeApplicationInstances(This,pVarApplicationInstanceID) \
    ( (This)->lpVtbl->ResumeApplicationInstances(This,pVarApplicationInstanceID) )
#define ICOMAdminCatalog2_RecycleApplicationInstances(This,pVarApplicationInstanceID,lReasonCode) \
    ( (This)->lpVtbl->RecycleApplicationInstances(This,pVarApplicationInstanceID,lReasonCode) )
#define ICOMAdminCatalog2_AreApplicationInstancesPaused(This,pVarApplicationInstanceID,pVarBoolPaused) \
    ( (This)->lpVtbl->AreApplicationInstancesPaused(This,pVarApplicationInstanceID,pVarBoolPaused) )
#define ICOMAdminCatalog2_DumpApplicationInstance(This,bstrApplicationInstanceID,bstrDirectory,lMaxImages,pbstrDumpFile) \
    ( (This)->lpVtbl->DumpApplicationInstance(This,bstrApplicationInstanceID,bstrDirectory,lMaxImages,pbstrDumpFile) )
#define ICOMAdminCatalog2_get_IsApplicationInstanceDumpSupported(This,pVarBoolDumpSupported) \
    ( (This)->lpVtbl->get_IsApplicationInstanceDumpSupported(This,pVarBoolDumpSupported) )
#define ICOMAdminCatalog2_CreateServiceForApplication(This,bstrApplicationIDOrName,bstrServiceName,bstrStartType,bstrErrorControl,bstrDependencies,bstrRunAs,bstrPassword,bDesktopOk) \
    ( (This)->lpVtbl->CreateServiceForApplication(This,bstrApplicationIDOrName,bstrServiceName,bstrStartType,bstrErrorControl,bstrDependencies,bstrRunAs,bstrPassword,bDesktopOk) )
#define ICOMAdminCatalog2_DeleteServiceForApplication(This,bstrApplicationIDOrName) \
    ( (This)->lpVtbl->DeleteServiceForApplication(This,bstrApplicationIDOrName) )
#define ICOMAdminCatalog2_GetPartitionID(This,bstrApplicationIDOrName,pbstrPartitionID) \
    ( (This)->lpVtbl->GetPartitionID(This,bstrApplicationIDOrName,pbstrPartitionID) )
#define ICOMAdminCatalog2_GetPartitionName(This,bstrApplicationIDOrName,pbstrPartitionName) \
    ( (This)->lpVtbl->GetPartitionName(This,bstrApplicationIDOrName,pbstrPartitionName) )
#define ICOMAdminCatalog2_put_CurrentPartition(This,bstrPartitionIDOrName) \
    ( (This)->lpVtbl->put_CurrentPartition(This,bstrPartitionIDOrName) )
#define ICOMAdminCatalog2_get_CurrentPartitionID(This,pbstrPartitionID) \
    ( (This)->lpVtbl->get_CurrentPartitionID(This,pbstrPartitionID) )
#define ICOMAdminCatalog2_get_CurrentPartitionName(This,pbstrPartitionName) \
    ( (This)->lpVtbl->get_CurrentPartitionName(This,pbstrPartitionName) )
#define ICOMAdminCatalog2_get_GlobalPartitionID(This,pbstrGlobalPartitionID) \
    ( (This)->lpVtbl->get_GlobalPartitionID(This,pbstrGlobalPartitionID) )
#define ICOMAdminCatalog2_FlushPartitionCache(This) \
    ( (This)->lpVtbl->FlushPartitionCache(This) )
#define ICOMAdminCatalog2_CopyApplications(This,bstrSourcePartitionIDOrName,pVarApplicationID,bstrDestinationPartitionIDOrName) \
    ( (This)->lpVtbl->CopyApplications(This,bstrSourcePartitionIDOrName,pVarApplicationID,bstrDestinationPartitionIDOrName) )
#define ICOMAdminCatalog2_CopyComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName) \
    ( (This)->lpVtbl->CopyComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName) )
#define ICOMAdminCatalog2_MoveComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName) \
    ( (This)->lpVtbl->MoveComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName) )
#define ICOMAdminCatalog2_AliasComponent(This,bstrSrcApplicationIDOrName,bstrCLSIDOrProgID,bstrDestApplicationIDOrName,bstrNewProgId,bstrNewClsid) \
    ( (This)->lpVtbl->AliasComponent(This,bstrSrcApplicationIDOrName,bstrCLSIDOrProgID,bstrDestApplicationIDOrName,bstrNewProgId,bstrNewClsid) )
#define ICOMAdminCatalog2_IsSafeToDelete(This,bstrDllName,pCOMAdminInUse) \
    ( (This)->lpVtbl->IsSafeToDelete(This,bstrDllName,pCOMAdminInUse) )
#define ICOMAdminCatalog2_ImportUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType) \
    ( (This)->lpVtbl->ImportUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType) )
#define ICOMAdminCatalog2_PromoteUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType) \
    ( (This)->lpVtbl->PromoteUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType) )
#define ICOMAdminCatalog2_ImportComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType) \
    ( (This)->lpVtbl->ImportComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType) )
#define ICOMAdminCatalog2_get_Is64BitCatalogServer(This,pbIs64Bit) \
    ( (This)->lpVtbl->get_Is64BitCatalogServer(This,pbIs64Bit) )
#define ICOMAdminCatalog2_ExportPartition(This,bstrPartitionIDOrName,bstrPartitionFileName,lOptions) \
    ( (This)->lpVtbl->ExportPartition(This,bstrPartitionIDOrName,bstrPartitionFileName,lOptions) )
#define ICOMAdminCatalog2_InstallPartition(This,bstrFileName,bstrDestDirectory,lOptions,bstrUserID,bstrPassword,bstrRSN) \
    ( (This)->lpVtbl->InstallPartition(This,bstrFileName,bstrDestDirectory,lOptions,bstrUserID,bstrPassword,bstrRSN) )
#define ICOMAdminCatalog2_QueryApplicationFile2(This,bstrApplicationFile,ppFilesForImport) \
    ( (This)->lpVtbl->QueryApplicationFile2(This,bstrApplicationFile,ppFilesForImport) )
#define ICOMAdminCatalog2_GetComponentVersionCount(This,bstrCLSIDOrProgID,plVersionCount) \
    ( (This)->lpVtbl->GetComponentVersionCount(This,bstrCLSIDOrProgID,plVersionCount) )

#endif /* COBJMACROS */

#ifdef __cplusplus
}
#endif
