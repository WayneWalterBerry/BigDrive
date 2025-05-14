#pragma once

#include <OleAuto.h>

static HRESULT VariantInit(VARIANT* pVar, LPWSTR sz)
{
    ::VariantInit(pVar);
    pVar->vt = VT_BSTR;
    pVar->bstrVal = ::SysAllocString(sz);
    return S_OK;
}
