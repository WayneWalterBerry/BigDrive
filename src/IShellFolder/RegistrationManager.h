// <copyright file="RegistrationManager.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include <CommCtrl.h>
#include <guiddef.h>

class RegistrationManager
{
    HRESULT RegisterShellFoldersFromRegistry();

    /// <summary>
    /// Register the shell folder with the given GUID.
    /// </summary>
    /// <param name="guid">Drive Guid</param>
    HRESULT RegisterShellFolder(GUID guid);
};
