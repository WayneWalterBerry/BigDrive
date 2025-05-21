// <copyright file="ETWManifestManagerExports.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "ETWManifestManagerExports.h"

extern "C" {

    HRESULT RegisterManifest()
    {
        return ETWManifestManager::RegisterManifest();
    }

    HRESULT UnregisterManifest()
    {
        return ETWManifestManager::UnregisterManifest();
    }

} // extern "C"