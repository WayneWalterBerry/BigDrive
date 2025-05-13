// <copyright file="COMAdmin.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"

// Shared
#include "..\Shared\EventLogger.h"

// Forward Declarations
class COMAdminCatalog;

class COMAdmin : public Dispatch
{

protected:

    COMAdminCatalog* m_pCOMAdminCatalog;

public:

    COMAdmin(COMAdminCatalog* pCOMAdminCatalog, LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
        m_pCOMAdminCatalog = pCOMAdminCatalog;
    }

    ~COMAdmin()
    {
    }
};