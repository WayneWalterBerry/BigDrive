// <copyright file="BigDriveShellFolderFactory.cpp" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

#include "pch.h"

#include "BigDriveShellFolderFactory.h"

// Define the static member outside the class
PIDLIST_ABSOLUTE BigDriveShellFolderFactory::s_pidlRoot = ILCreateFromPathW(L"::");