// ICOMAdminCatalog.h
// C++ interface definition extracted from ComAdmin.h

#pragma once

#include <windows.h>
#include <unknwn.h>
#include <oaidl.h>

// UUID definition for ICOMAdminCatalog
// {DD662187-DFC2-11d1-A2CF-00805FC79235}
DEFINE_GUID(IID_ICOMAdminCatalog,
    0xDD662187, 0xDFC2, 0x11d1, 0xA2, 0xCF, 0x00, 0x80, 0x5F, 0xC7, 0x92, 0x35);

#ifdef __cplusplus

// C++ interface definition for ICOMAdminCatalog
#undef INTERFACE
#define INTERFACE ICOMAdminCatalog

DECLARE_INTERFACE_(ICOMAdminCatalog, IDispatch)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // IDispatch methods
    STDMETHOD(GetTypeInfoCount)(THIS_ UINT * pctinfo) PURE;
    STDMETHOD(GetTypeInfo)(THIS_ UINT iTInfo, LCID lcid, ITypeInfo * *ppTInfo) PURE;
    STDMETHOD(GetIDsOfNames)(THIS_ REFIID riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId) PURE;
    STDMETHOD(Invoke)(THIS_ DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pDispParams, VARIANT * pVarResult, EXCEPINFO * pExcepInfo, UINT * puArgErr) PURE;

    // ICOMAdminCatalog methods
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
};

#endif // __cplusplus
