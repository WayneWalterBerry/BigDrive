#pragma once

// Local
#include "Dispatch.h"

// Shared
#include "..\Shared\EventLogger.h"

class CatalogCollection : public Dispatch
{
protected:

    /// <summary>
    /// Static instance of EventLogger for logging events.
    /// </summary>
    static EventLogger s_eventLogger;

public:
    CatalogCollection(LPDISPATCH pDispatch)
        : Dispatch(pDispatch)
    {
    }
    ~CatalogCollection()
    {
    }

    HRESULT Populate(LPDISPATCH pIDispatch);
};
