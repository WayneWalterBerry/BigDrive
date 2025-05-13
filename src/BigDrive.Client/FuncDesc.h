#pragma once

// System
#include <oaidl.h>
#include <comutil.h> // For _bstr_t and COM utilities
#include <sstream>   // For std::ostringstream
#include <iomanip>   // For std::hex

class FuncDesc
{

private:

    ITypeInfo* m_pTypeInfo;
    FUNCDESC* m_pFuncDesc;



    FuncDesc(ITypeInfo* pTypeInfo, FUNCDESC* pFuncDesc)
        : m_pFuncDesc(pFuncDesc)
    {
        m_pTypeInfo = pTypeInfo;

        if (m_pTypeInfo)
        {
            m_pTypeInfo->AddRef();
        }
    }

public:

    ~FuncDesc()
    {
        if (m_pFuncDesc)
        {
            m_pTypeInfo->ReleaseFuncDesc(m_pFuncDesc);
            m_pFuncDesc = nullptr;
        }

        if (m_pTypeInfo)
        {
            m_pTypeInfo->Release();
            m_pTypeInfo = nullptr;
        }
    }

    static HRESULT Create(ITypeInfo* pTypeInfo, UINT i, FuncDesc **pFuncDesc)
    {
        FUNCDESC* p;

        HRESULT hr = pTypeInfo->GetFuncDesc(i, &p);

        if (FAILED(hr))
        {
            // Failed to get function description
            return hr;
        }

        *pFuncDesc = new FuncDesc(pTypeInfo, p);
        if (!*pFuncDesc)
        {
            // Memory allocation failure
            pTypeInfo->ReleaseFuncDesc(p);
            return E_OUTOFMEMORY;
        }

        return hr;
    }

    HRESULT GetName(BSTR* pbstrName)
    {
        UINT cNames = 0;
        return m_pTypeInfo->GetNames(m_pFuncDesc->memid, pbstrName, 1, &cNames);
    }


    HRESULT InvkindToBSTR(BSTR* pOutput)
    {
        if (!pOutput) return E_POINTER; // Null output parameter check

        BSTR result = nullptr;
        switch (m_pFuncDesc->invkind)
        {
        case INVOKE_FUNC: result = SysAllocString(L"Method (Function Call)"); break;
        case INVOKE_PROPERTYGET: result = SysAllocString(L"Property Getter"); break;
        case INVOKE_PROPERTYPUT: result = SysAllocString(L"Property Setter"); break;
        case INVOKE_PROPERTYPUTREF: result = SysAllocString(L"Property Setter (Reference)"); break;
        default: result = SysAllocString(L"Unknown INVOKEKIND"); break;
        }

        if (!result) return E_OUTOFMEMORY; // Memory allocation failure check

        *pOutput = result;
        return S_OK;
    }

    HRESULT FunckindToBSTR(BSTR* pOutput)
    {
        if (!pOutput) return E_POINTER; // Null output parameter check

        BSTR result = nullptr;
        switch (m_pFuncDesc->funckind)
        {
        case FUNC_VIRTUAL: result = SysAllocString(L"Virtual Function"); break;
        case FUNC_PUREVIRTUAL: result = SysAllocString(L"Pure Virtual Function"); break;
        case FUNC_NONVIRTUAL: result = SysAllocString(L"Non-Virtual Function"); break;
        case FUNC_STATIC: result = SysAllocString(L"Static Function"); break;
        case FUNC_DISPATCH: result = SysAllocString(L"IDispatch Function"); break;
        default: result = SysAllocString(L"Unknown FUNCKIND"); break;
        }

        if (!result) return E_OUTOFMEMORY; // Memory allocation failure check

        *pOutput = result;
        return S_OK;
    }

    HRESULT VtToBSTR(VARTYPE vt, BSTR* pOutput)
    {
        if (!pOutput)
        {
            return E_POINTER; // Null output parameter check
        }

        BSTR result = nullptr;

        switch (vt)
        {
        case VT_EMPTY: result = SysAllocString(L"VT_EMPTY"); break;
        case VT_NULL: result = SysAllocString(L"VT_NULL"); break;
        case VT_I2: result = SysAllocString(L"VT_I2"); break;
        case VT_I4: result = SysAllocString(L"VT_I4"); break;
        case VT_R4: result = SysAllocString(L"VT_R4"); break;
        case VT_R8: result = SysAllocString(L"VT_R8"); break;
        case VT_CY: result = SysAllocString(L"VT_CY"); break;
        case VT_DATE: result = SysAllocString(L"VT_DATE"); break;
        case VT_BSTR: result = SysAllocString(L"VT_BSTR"); break;
        case VT_DISPATCH: result = SysAllocString(L"VT_DISPATCH"); break;
        case VT_ERROR: result = SysAllocString(L"VT_ERROR"); break;
        case VT_BOOL: result = SysAllocString(L"VT_BOOL"); break;
        case VT_VARIANT: result = SysAllocString(L"VT_VARIANT"); break;
        case VT_UNKNOWN: result = SysAllocString(L"VT_UNKNOWN"); break;
        case VT_DECIMAL: result = SysAllocString(L"VT_DECIMAL"); break;
        case VT_I1: result = SysAllocString(L"VT_I1"); break;
        case VT_UI1: result = SysAllocString(L"VT_UI1"); break;
        case VT_UI2: result = SysAllocString(L"VT_UI2"); break;
        case VT_UI4: result = SysAllocString(L"VT_UI4"); break;
        case VT_I8: result = SysAllocString(L"VT_I8"); break;
        case VT_UI8: result = SysAllocString(L"VT_UI8"); break;
        case VT_INT: result = SysAllocString(L"VT_INT"); break;
        case VT_UINT: result = SysAllocString(L"VT_UINT"); break;
        case VT_VOID: result = SysAllocString(L"VT_VOID"); break;
        case VT_HRESULT: result = SysAllocString(L"VT_HRESULT"); break;
        case VT_PTR: result = SysAllocString(L"VT_PTR"); break;
        case VT_SAFEARRAY: result = SysAllocString(L"VT_SAFEARRAY"); break;
        case VT_CARRAY: result = SysAllocString(L"VT_CARRAY"); break;
        case VT_USERDEFINED: result = SysAllocString(L"VT_USERDEFINED"); break;
        case VT_LPSTR: result = SysAllocString(L"VT_LPSTR"); break;
        case VT_LPWSTR: result = SysAllocString(L"VT_LPWSTR"); break;
        case VT_RECORD: result = SysAllocString(L"VT_RECORD"); break;
        case VT_INT_PTR: result = SysAllocString(L"VT_INT_PTR"); break;
        case VT_UINT_PTR: result = SysAllocString(L"VT_UINT_PTR"); break;
        case VT_FILETIME: result = SysAllocString(L"VT_FILETIME"); break;
        case VT_BLOB: result = SysAllocString(L"VT_BLOB"); break;
        case VT_STREAM: result = SysAllocString(L"VT_STREAM"); break;
        case VT_STORAGE: result = SysAllocString(L"VT_STORAGE"); break;
        case VT_STREAMED_OBJECT: result = SysAllocString(L"VT_STREAMED_OBJECT"); break;
        case VT_STORED_OBJECT: result = SysAllocString(L"VT_STORED_OBJECT"); break;
        case VT_BLOB_OBJECT: result = SysAllocString(L"VT_BLOB_OBJECT"); break;
        case VT_CF: result = SysAllocString(L"VT_CF"); break;
        case VT_CLSID: result = SysAllocString(L"VT_CLSID"); break;
        case VT_VERSIONED_STREAM: result = SysAllocString(L"VT_VERSIONED_STREAM"); break;
        default: result = SysAllocString(L"Unknown VARTYPE"); break;
        }

        if (!result)
        {
            return E_OUTOFMEMORY; // Memory allocation failure
        }

        *pOutput = result;
        return S_OK;
    }

    HRESULT Serialize(BSTR& bstrJson)
    {
        HRESULT hr = S_OK;

        BSTR bstrName;
        hr = GetName(&bstrName);
        if (FAILED(hr))
        {
            return hr;
        }

        BSTR bstrFunctionKind;
        hr = FunckindToBSTR(&bstrFunctionKind);
        if (FAILED(hr))
        {
            return hr;
        }

        BSTR bstrInvkind;
        hr = InvkindToBSTR(&bstrInvkind);
        if (FAILED(hr))
        {
            return hr;
        }

        BSTR bstrVt;
        hr = VtToBSTR(m_pFuncDesc->elemdescFunc.tdesc.vt, &bstrVt);
        if (FAILED(hr))
        {
            return hr;
        }

        try
        {
            std::ostringstream jsonStream;

            // Start JSON object
            jsonStream << "{";

            // Serialize each property of FUNCDESC
            jsonStream << "\"Name\": \"" << _bstr_t(bstrName) << "\",";
            jsonStream << "\"memid\": " << m_pFuncDesc->memid << ",";
            jsonStream << "\"funckind\": \"" << _bstr_t(bstrFunctionKind) << "\",";
            jsonStream << "\"invkind\": \"" << _bstr_t(bstrInvkind) << "\",";
            jsonStream << "\"callconv\": " << m_pFuncDesc->callconv << ",";
            jsonStream << "\"cParams\": " << m_pFuncDesc->cParams << ",";
            jsonStream << "\"cParamsOpt\": " << m_pFuncDesc->cParamsOpt << ",";
            jsonStream << "\"oVft\": " << m_pFuncDesc->oVft << ",";
            jsonStream << "\"cScodes\": " << m_pFuncDesc->cScodes << ",";
            jsonStream << "\"wFuncFlags\": " << m_pFuncDesc->wFuncFlags;

            // Serialize elemdescFunc (nested structure)
            jsonStream << ",\"elemdescFunc\": {";
            jsonStream << "\"tdesc\": {";
            jsonStream << "\"vt\": \"" << _bstr_t(bstrVt) << "\"";
            jsonStream << "},";
            jsonStream << "\"idldesc\": {";
            jsonStream << "\"dwReserved\": " << m_pFuncDesc->elemdescFunc.idldesc.dwReserved << ",";
            jsonStream << "\"wIDLFlags\": " << m_pFuncDesc->elemdescFunc.idldesc.wIDLFlags;
            jsonStream << "}";
            jsonStream << "}";

            // Serialize lprgelemdescParam as a nested JSON array
            jsonStream << ",\"parameters\": [";
            for (SHORT i = 0; i < m_pFuncDesc->cParams; ++i)
            {
                ELEMDESC& elemDesc = m_pFuncDesc->lprgelemdescParam[i];

                BSTR bstrElementVt;
                hr = VtToBSTR(elemDesc.tdesc.vt, &bstrElementVt);
                if (FAILED(hr))
                {
                    return hr;
                }

                jsonStream << "{";
                jsonStream << "\"tdesc\": {";
                jsonStream << "\"vt\": \"" << _bstr_t(bstrElementVt) << "\"";
                jsonStream << "},";
                jsonStream << "\"idldesc\": {";
                jsonStream << "\"dwReserved\": " << elemDesc.idldesc.dwReserved << ",";
                jsonStream << "\"wIDLFlags\": " << elemDesc.idldesc.wIDLFlags;
                jsonStream << "}";
                jsonStream << "}";

                if (i < m_pFuncDesc->cParams - 1)
                {
                    jsonStream << ",";
                }

                ::SysFreeString(bstrElementVt);
            }
            jsonStream << "]";

            // End JSON object
            jsonStream << "}";

            // Convert JSON string to BSTR
            std::string jsonString = jsonStream.str();
            _bstr_t bstr(jsonString.c_str());
            bstrJson = bstr.Detach();

            return S_OK;
        }
        catch (const std::exception&)
        {
            // Handle any exceptions and return failure
            return E_FAIL;
        }

        ::SysFreeString(bstrName);
        ::SysFreeString(bstrFunctionKind);
        ::SysFreeString(bstrInvkind);
    }

};

