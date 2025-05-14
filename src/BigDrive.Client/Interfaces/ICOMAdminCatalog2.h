// ICOMAdminCatalog2.h
// C++ interface definition extracted from ComAdmin.h

#pragma once

#include <windows.h>
#include <unknwn.h>
#include <oaidl.h>

#include "ICOMAdminCatalog.h"

// Forward declaration for ICOMAdminCatalog
#ifndef __ICOMAdminCatalog_FWD_DEFINED__
#define __ICOMAdminCatalog_FWD_DEFINED__

typedef interface ICOMAdminCatalog ICOMAdminCatalog;
#endif /* __ICOMAdminCatalog_FWD_DEFINED__ */

// Forward declaration for COMAdminInUse enum
typedef enum COMAdminInUse {
    COMAdminNotInUse = 0,
    COMAdminInUseByCatalog = 0x1,
    COMAdminInUseByRegistryUnknown = 0x2,
    COMAdminInUseByRegistryProxyStub = 0x3,
    COMAdminInUseByRegistryTypeLib = 0x4,
    COMAdminInUseByRegistryClsid = 0x5
} COMAdminInUse;

// UUID definition for ICOMAdminCatalog2
// {790C6E0B-9194-4cc9-9426-A48A63185696}
DEFINE_GUID(IID_ICOMAdminCatalog2,
    0x790C6E0B, 0x9194, 0x4cc9, 0x94, 0x26, 0xA4, 0x8A, 0x63, 0x18, 0x56, 0x96);

#ifdef __cplusplus

// C++ interface definition for ICOMAdminCatalog2
#undef INTERFACE
#define INTERFACE ICOMAdminCatalog2

DECLARE_INTERFACE_(ICOMAdminCatalog2, ICOMAdminCatalog)
{
    // Inherited methods from IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Inherited methods from IDispatch
    STDMETHOD(GetTypeInfoCount)(THIS_ UINT * pctinfo) PURE;
    STDMETHOD(GetTypeInfo)(THIS_ UINT iTInfo, LCID lcid, ITypeInfo * *ppTInfo) PURE;
    STDMETHOD(GetIDsOfNames)(THIS_ REFIID riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId) PURE;
    STDMETHOD(Invoke)(THIS_ DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pDispParams, VARIANT * pVarResult, EXCEPINFO * pExcepInfo, UINT * puArgErr) PURE;

    // Inherited methods from ICOMAdminCatalog
    STDMETHOD(GetCollection)(THIS_ BSTR bstrCollName, IDispatch * *ppCatalogCollection) PURE;
    STDMETHOD(Connect)(THIS_ BSTR bstrCatalogServerName, IDispatch * *ppCatalogCollection) PURE;
    STDMETHOD(get_MajorVersion)(THIS_ long* plMajorVersion) PURE;
    STDMETHOD(get_MinorVersion)(THIS_ long* plMinorVersion) PURE;
    STDMETHOD(GetCollectionByQuery)(THIS_ BSTR bstrCollName, SAFEARRAY * *ppsaVarQuery, IDispatch * *ppCatalogCollection) PURE;
    STDMETHOD(ImportComponent)(THIS_ BSTR bstrApplIDOrName, BSTR bstrCLSIDOrProgID) PURE;
    STDMETHOD(InstallComponent)(THIS_ BSTR bstrApplIDOrName, BSTR bstrDLL, BSTR bstrTLB, BSTR bstrPSDLL) PURE;
    STDMETHOD(ShutdownApplication)(THIS_ BSTR bstrApplIDOrName) PURE;
    STDMETHOD(ExportApplication)(THIS_ BSTR bstrApplIDOrName, BSTR bstrApplicationFile, long lOptions) PURE;
    STDMETHOD(InstallApplication)(THIS_ BSTR bstrApplicationFile, BSTR bstrDestinationDirectory, long lOptions, BSTR bstrUserId, BSTR bstrPassword, BSTR bstrRSN) PURE;
    STDMETHOD(StopRouter)(THIS) PURE;
    STDMETHOD(RefreshRouter)(THIS) PURE;
    STDMETHOD(StartRouter)(THIS) PURE;
    STDMETHOD(Reserved1)(THIS) PURE;
    STDMETHOD(Reserved2)(THIS) PURE;
    STDMETHOD(InstallMultipleComponents)(THIS_ BSTR bstrApplIDOrName, SAFEARRAY * *ppsaVarFileNames, SAFEARRAY * *ppsaVarCLSIDs) PURE;
    STDMETHOD(GetMultipleComponentsInfo)(THIS_ BSTR bstrApplIdOrName, SAFEARRAY * *ppsaVarFileNames, SAFEARRAY * *ppsaVarCLSIDs, SAFEARRAY * *ppsaVarClassNames, SAFEARRAY * *ppsaVarFileFlags, SAFEARRAY * *ppsaVarComponentFlags) PURE;
    STDMETHOD(RefreshComponents)(THIS) PURE;
    STDMETHOD(BackupREGDB)(THIS_ BSTR bstrBackupFilePath) PURE;
    STDMETHOD(RestoreREGDB)(THIS_ BSTR bstrBackupFilePath) PURE;
    STDMETHOD(QueryApplicationFile)(THIS_ BSTR bstrApplicationFile, BSTR * pbstrApplicationName, BSTR * pbstrApplicationDescription, VARIANT_BOOL * pbHasUsers, VARIANT_BOOL * pbIsProxy, SAFEARRAY * *ppsaVarFileNames) PURE;
    STDMETHOD(StartApplication)(THIS_ BSTR bstrApplIdOrName) PURE;
    STDMETHOD(ServiceCheck)(THIS_ long lService, long* plStatus) PURE;
    STDMETHOD(InstallMultipleEventClasses)(THIS_ BSTR bstrApplIdOrName, SAFEARRAY * *ppsaVarFileNames, SAFEARRAY * *ppsaVarCLSIDS) PURE;
    STDMETHOD(InstallEventClass)(THIS_ BSTR bstrApplIdOrName, BSTR bstrDLL, BSTR bstrTLB, BSTR bstrPSDLL) PURE;
    STDMETHOD(GetEventClassesForIID)(THIS_ BSTR bstrIID, SAFEARRAY * *ppsaVarCLSIDs, SAFEARRAY * *ppsaVarProgIDs, SAFEARRAY * *ppsaVarDescriptions) PURE;

    // Methods specific to ICOMAdminCatalog2
    STDMETHOD(GetCollectionByQuery2)(THIS_ BSTR bstrCollectionName, VARIANT * pVarQueryStrings, IDispatch * *ppCatalogCollection) PURE;
    STDMETHOD(GetApplicationInstanceIDFromProcessID)(THIS_ long lProcessID, BSTR * pbstrApplicationInstanceID) PURE;
    STDMETHOD(ShutdownApplicationInstances)(THIS_ VARIANT * pVarApplicationInstanceID) PURE;
    STDMETHOD(PauseApplicationInstances)(THIS_ VARIANT * pVarApplicationInstanceID) PURE;
    STDMETHOD(ResumeApplicationInstances)(THIS_ VARIANT * pVarApplicationInstanceID) PURE;
    STDMETHOD(RecycleApplicationInstances)(THIS_ VARIANT * pVarApplicationInstanceID, long lReasonCode) PURE;
    STDMETHOD(AreApplicationInstancesPaused)(THIS_ VARIANT * pVarApplicationInstanceID, VARIANT_BOOL * pVarBoolPaused) PURE;
    STDMETHOD(DumpApplicationInstance)(THIS_ BSTR bstrApplicationInstanceID, BSTR bstrDirectory, long lMaxImages, BSTR * pbstrDumpFile) PURE;
    STDMETHOD(get_IsApplicationInstanceDumpSupported)(THIS_ VARIANT_BOOL * pVarBoolDumpSupported) PURE;
    STDMETHOD(CreateServiceForApplication)(THIS_ BSTR bstrApplicationIDOrName, BSTR bstrServiceName, BSTR bstrStartType, BSTR bstrErrorControl, BSTR bstrDependencies, BSTR bstrRunAs, BSTR bstrPassword, VARIANT_BOOL bDesktopOk) PURE;
    STDMETHOD(DeleteServiceForApplication)(THIS_ BSTR bstrApplicationIDOrName) PURE;
    STDMETHOD(GetPartitionID)(THIS_ BSTR bstrApplicationIDOrName, BSTR * pbstrPartitionID) PURE;
    STDMETHOD(GetPartitionName)(THIS_ BSTR bstrApplicationIDOrName, BSTR * pbstrPartitionName) PURE;
    STDMETHOD(put_CurrentPartition)(THIS_ BSTR bstrPartitionIDOrName) PURE;
    STDMETHOD(get_CurrentPartitionID)(THIS_ BSTR * pbstrPartitionID) PURE;
    STDMETHOD(get_CurrentPartitionName)(THIS_ BSTR * pbstrPartitionName) PURE;
    STDMETHOD(get_GlobalPartitionID)(THIS_ BSTR * pbstrGlobalPartitionID) PURE;
    STDMETHOD(FlushPartitionCache)(THIS) PURE;
    STDMETHOD(CopyApplications)(THIS_ BSTR bstrSourcePartitionIDOrName, VARIANT * pVarApplicationID, BSTR bstrDestinationPartitionIDOrName) PURE;
    STDMETHOD(CopyComponents)(THIS_ BSTR bstrSourceApplicationIDOrName, VARIANT * pVarCLSIDOrProgID, BSTR bstrDestinationApplicationIDOrName) PURE;
    STDMETHOD(MoveComponents)(THIS_ BSTR bstrSourceApplicationIDOrName, VARIANT * pVarCLSIDOrProgID, BSTR bstrDestinationApplicationIDOrName) PURE;
    STDMETHOD(AliasComponent)(THIS_ BSTR bstrSrcApplicationIDOrName, BSTR bstrCLSIDOrProgID, BSTR bstrDestApplicationIDOrName, BSTR bstrNewProgId, BSTR bstrNewClsid) PURE;
    STDMETHOD(IsSafeToDelete)(THIS_ BSTR bstrDllName, COMAdminInUse * pCOMAdminInUse) PURE;
    STDMETHOD(ImportUnconfiguredComponents)(THIS_ BSTR bstrApplicationIDOrName, VARIANT * pVarCLSIDOrProgID, VARIANT * pVarComponentType) PURE;
    STDMETHOD(PromoteUnconfiguredComponents)(THIS_ BSTR bstrApplicationIDOrName, VARIANT * pVarCLSIDOrProgID, VARIANT * pVarComponentType) PURE;
    STDMETHOD(ImportComponents)(THIS_ BSTR bstrApplicationIDOrName, VARIANT * pVarCLSIDOrProgID, VARIANT * pVarComponentType) PURE;
    STDMETHOD(get_Is64BitCatalogServer)(THIS_ VARIANT_BOOL * pbIs64Bit) PURE;
    STDMETHOD(ExportPartition)(THIS_ BSTR bstrPartitionIDOrName, BSTR bstrPartitionFileName, long lOptions) PURE;
    STDMETHOD(InstallPartition)(THIS_ BSTR bstrFileName, BSTR bstrDestDirectory, long lOptions, BSTR bstrUserID, BSTR bstrPassword, BSTR bstrRSN) PURE;
    STDMETHOD(QueryApplicationFile2)(THIS_ BSTR bstrApplicationFile, IDispatch * *ppFilesForImport) PURE;
    STDMETHOD(GetComponentVersionCount)(THIS_ BSTR bstrCLSIDOrProgID, long* plVersionCount) PURE;
};

#endif // __cplusplus


