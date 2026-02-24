================================================================================
                        BigDrive.Interfaces Project
                         Architecture Documentation
================================================================================

OVERVIEW
--------------------------------------------------------------------------------
BigDrive.Interfaces defines the COM-visible interface contracts that enable
communication between the native C++ shell extension (BigDrive.ShellFolder)
and managed .NET provider implementations. This project serves as the
contract layer in BigDrive's cross-language, cross-process architecture.

ARCHITECTURAL ROLE
--------------------------------------------------------------------------------
BigDrive uses a shim architecture to integrate custom storage providers with
Windows Explorer:

    +-------------------+       COM+        +----------------------+
    |  explorer.exe     |  <=============>  |  Provider (C#/.NET)  |
    |  (BigDrive.       |   Out-of-Process  |  Implements these    |
    |   ShellFolder)    |   COM Interop     |  interfaces          |
    +-------------------+                   +----------------------+
            ^                                         |
            |                                         |
            +--------------- References ---------------+
                        BigDrive.Interfaces

Key Benefits:
  - Process Isolation: Providers run outside explorer.exe, preventing crashes
    from affecting the Windows shell.
  - Language Flexibility: Providers can be written in C# or any COM-compatible
    language.
  - Extensibility: Third-party developers implement these interfaces to create
    custom virtual drives.

INTERFACE DEFINITIONS
--------------------------------------------------------------------------------

IBigDriveEnumerate (457ED786-889A-4C16-A6E5-6A25013D0AFA)
  Purpose: Enumerate folders and files within a virtual drive.
  Methods:
    - EnumerateFolders(driveGuid, path) -> string[]
    - EnumerateFiles(driveGuid, path) -> string[]

IBigDriveFileInfo (A98A0D26-4D5D-4B50-B6FF-8BCB360CB066)
  Purpose: Retrieve file metadata.
  Methods:
    - LastModifiedTime(driveGuid, path) -> DateTime
    - GetFileSize(driveGuid, path) -> ulong

IBigDriveFileOperations (7BE23F90-8D32-4D88-B4E7-59BFDA941F04)
  Purpose: Perform file and folder operations.
  Methods:
    - CopyFileToBigDrive(driveGuid, localFilePath, bigDriveTargetPath)
    - CopyFileFromBigDrive(driveGuid, bigDriveFilePath, localTargetPath)
    - DeleteFile(driveGuid, bigDriveFilePath)
    - CreateDirectory(driveGuid, bigDriveDirectoryPath)
    - OpenFile(driveGuid, bigDriveFilePath, hwndParent)
    - MoveFile(driveGuid, sourcePath, destinationPath)

IBigDriveFileData (0F471AE9-1787-437F-B230-60CA6717DD04)
  Purpose: Retrieve file content as a COM IStream.
  Methods:
    - GetFileData(driveGuid, path, out IStream stream) -> HRESULT

COM INTEROP DESIGN
--------------------------------------------------------------------------------
All interfaces follow these patterns for cross-language compatibility:

  1. [ComVisible(true)] - Exposes the interface to COM clients.
  2. [Guid("...")] - Unique IID for each interface.
  3. [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)] - Pure IUnknown-based
     interface for maximum compatibility with C++ clients.
  4. Drive Identification: All methods accept a driveGuid parameter to identify
     which registered drive the operation targets.

NATIVE C++ COUNTERPARTS
--------------------------------------------------------------------------------
The BigDrive.Client project contains C++ header equivalents of these interfaces
for use by the shell extension:

  - IBigDriveEnumerate.h
  - IBigDriveFileInfo.h
  - IBigDriveFileOperations.h
  - IBigDriveFileData.h
  - IBigDriveConfiguration.h

These headers define identical IIDs and method signatures, enabling seamless
QueryInterface calls across the COM boundary.

IMPLEMENTING A PROVIDER
--------------------------------------------------------------------------------
To create a custom BigDrive provider:

  1. Reference BigDrive.Interfaces in your .NET project.
  2. Create a class that inherits from ServicedComponent (for COM+ hosting).
  3. Implement the required interfaces:
       - IBigDriveEnumerate (required)
       - IBigDriveFileInfo (required)
       - IBigDriveFileOperations (optional)
       - IBigDriveFileData (optional)
  4. Register the COM+ application with the BigDrive service.

Example: See BigDrive.Provider.Sample for a complete implementation.

TARGET FRAMEWORK
--------------------------------------------------------------------------------
.NET Framework 4.7.2

This framework version is required for System.EnterpriseServices (COM+) support
and COM interop compatibility with the native shell extension.

================================================================================
                    Copyright (c) Wayne Walter Berry. All rights reserved.
================================================================================
