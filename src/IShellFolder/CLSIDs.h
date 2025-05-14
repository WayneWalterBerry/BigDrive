#pragma once

#include <initguid.h> // Required for DEFINE_GUID

// CLSID for BigDriveFolder
// {D4E5F6A7-B8C9-0123-4567-89ABCDEF1234}
DEFINE_GUID(CLSID_BigDriveShellFolder,
  0xD4E5F6A7, 0xB8C9, 0x0123, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x12, 0x34);

// IID for IShellFolder (already defined in system headers, but custom interfaces would go here)
