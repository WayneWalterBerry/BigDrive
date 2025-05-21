// <copyright file="ETWManifestManagerExports.h" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#pragma once

#include "..\ETWManifestManager.h"

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) HRESULT RegisterManifest();
    __declspec(dllexport) HRESULT UnregisterManifest();

#ifdef __cplusplus
}
#endif