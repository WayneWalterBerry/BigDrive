#pragma once

#include <windows.h>
#include <shtypes.h>

class ItemIdList
{
private:

    LPITEMIDLIST m_pidl;

public:

    ItemIdList(LPITEMIDLIST pidl)
        : m_pidl(pidl)
    {
    }
};