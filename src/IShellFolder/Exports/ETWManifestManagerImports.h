// <copyright file="ETWManifestManagerImports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\ETWManifestManager.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) HRESULT RegisterManifest();
    __declspec(dllimport) HRESULT UnregisterManifest();

#ifdef __cplusplus
}
#endif