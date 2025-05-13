// <copyright file="COMAdmin.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

// Local
#include "Dispatch.h"

// Shared
#include "..\Shared\EventLogger.h"

class COMAdmin : public Dispatch
{

protected:


public:

    COMAdmin(LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
    }

    ~COMAdmin()
    {
    }
};