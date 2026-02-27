# BigDrive Components

This document provides a detailed breakdown of all BigDrive components, their purposes, platform architectures, and execution models.

---

## Platform Architecture

BigDrive consists of **platform-specific C++ components** and **platform-agnostic C# components**.

---

## C++ Components (Platform-Specific: x86 and x64)

These components integrate with `explorer.exe` and Windows shell APIs, which are architecture-dependent. They must be built for both 32-bit (x86) and 64-bit (x64) architectures to support both 32-bit and 64-bit Windows Explorer processes.

### BigDrive.ShellFolder.dll

| Property | Value |
|----------|-------|
| **Type** | Shell Extension (IShellFolder namespace extension) |
| **Platform** | x86, x64 |
| **Process** | Loads into `explorer.exe` |
| **Location** | `src/BigDrive.ShellFolder/` |

**Purpose:**
- Implements Windows Explorer namespace extension
- Provides IShellFolder2 interface for folder enumeration
- Handles drag-and-drop operations via IDropTarget
- Provides file icons via IExtractIcon
- Enables file copy/move via ITransferSource

**Why Platform-Specific:**
- Loads directly into `explorer.exe` (which can be 32-bit or 64-bit)
- Must match the bitness of the host Explorer process
- Uses architecture-specific Windows shell APIs (SHCreateShellItem, etc.)

### BigDrive.Extension.dll

| Property | Value |
|----------|-------|
| **Type** | Context Menu Handler (IContextMenu) |
| **Platform** | x86, x64 |
| **Process** | Loads into `explorer.exe` |
| **Location** | `src/BigDrive.Extension/` |

**Purpose:**
- Provides right-click context menu integration in Windows Explorer
- Adds "Map BigDrive..." menu item to folders
- Launches drive mapping dialog

**Why Platform-Specific:**
- Loads directly into `explorer.exe`
- Must match Explorer's bitness
- Direct shell API integration

### BigDrive.Client.dll

| Property | Value |
|----------|-------|
| **Type** | COM+ Client Library |
| **Platform** | x86, x64 |
| **Process** | Static/dynamic library |
| **Location** | `src/BigDrive.Client/` |

**Purpose:**
- C++ library for managing providers and drives via COM+
- Wrapper around COM+ Catalog API for application management
- Used by C++ components to interact with COM+ services

**Why Platform-Specific:**
- Called by platform-specific shell extensions
- Must match caller's bitness
- Compiled as static library for x64 Debug builds

### Test Projects

#### BigDrive.Client.Test

| Property | Value |
|----------|-------|
| **Type** | Unit Test Library |
| **Platform** | x86, x64 |
| **Location** | `test/unit/BigDrive.Client.Test/` |

Unit tests for BigDrive.Client functionality.

#### BigDrive.ShellFolder.Test

| Property | Value |
|----------|-------|
| **Type** | Unit Test Library |
| **Platform** | x86, x64 |
| **Location** | `test/unit/BigDrive.ShellFolder.Test/` |

Unit tests for BigDrive.ShellFolder functionality.

---

## C# Components (Any CPU)

These components run in COM+ applications (`dllhost.exe`) or as standalone .NET processes. They target "Any CPU" and are JIT-compiled to the appropriate architecture at runtime.

### User-Facing Components

#### BigDrive.Shell.exe

| Property | Value |
|----------|-------|
| **Type** | Console Application |
| **Platform** | Any CPU |
| **Process** | Standalone .NET process |
| **Location** | `src/BigDrive.Shell/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- Command-line interface for BigDrive operations
- Commands: `dir`, `cd`, `copy`, `move`, `del`, `mount`, `unmount`, `providers`, `drives`
- Alternative to Windows Explorer for BigDrive access

**Key Features:**
- Path parsing and resolution (supports `..`, `.`, absolute/relative paths)
- Drive letter mapping (Z: → DriveGuid)
- Provider activation via COM+
- File transfer between local and BigDrive storage

**Dependencies:**
- BigDrive.Interfaces (COM interface definitions)
- BigDrive.Service.Interfaces (provisioning interface)
- BigDrive.ConfigProvider (registry access)

---

### Service Components

#### BigDrive.Service.dll

| Property | Value |
|----------|-------|
| **Type** | ServicedComponent (COM+) |
| **Platform** | Any CPU |
| **Process** | COM+ Application in `dllhost.exe` |
| **Location** | `src/BigDrive.Service/` |
| **Framework** | .NET Framework 4.7.2 |
| **Identity** | BigDriveInstaller (elevated local service account) |

**Purpose:**
- Drive provisioning and deprovisioning with elevated registry access
- Implements `IBigDriveProvision` interface
- Writes to HKCR (COM class registration)
- Writes to HKLM shell namespace (MyComputer\NameSpace)
- Writes to HKLM\SOFTWARE\BigDrive
- Refreshes Explorer after drive changes

**Key Methods:**
- `Create(Guid driveGuid)` — Create drive from existing registry config
- `CreateFromConfiguration(string json)` — Create drive from JSON
- `UnmountDrive(Guid driveGuid)` — Remove drive and refresh Explorer

**Security Model:**
- Runs under elevated BigDriveInstaller identity
- Shell must **never** write to registry directly
- Shell calls service via COM+ out-of-process activation

---

### Storage Provider Components

All providers run in separate COM+ applications under the **Interactive User** identity.

#### BigDrive.Provider.Flickr.dll

| Property | Value |
|----------|-------|
| **Type** | ServicedComponent (COM+) |
| **Platform** | Any CPU |
| **Process** | COM+ Application in `dllhost.exe` |
| **Location** | `src/BigDrive.Provider.Flickr/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- Flickr API integration
- Photo set enumeration as folders
- Photo download/upload
- OAuth authentication

**Implemented Interfaces:**
- IBigDriveEnumerate (required)
- IBigDriveFileInfo (optional)
- IBigDriveFileData (optional)
- IBigDriveAuthentication (optional)

#### BigDrive.Provider.Archive.dll

| Property | Value |
|----------|-------|
| **Type** | ServicedComponent (COM+) |
| **Platform** | Any CPU |
| **Process** | COM+ Application in `dllhost.exe` |
| **Location** | `src/BigDrive.Provider.Archive/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- Archive file access (tar, tar.gz, tar.bz2, tar.xz, etc.)
- Read-only access to archive contents
- Virtual folder structure from archive entries

#### BigDrive.Provider.Zip.dll

| Property | Value |
|----------|-------|
| **Type** | ServicedComponent (COM+) |
| **Platform** | Any CPU |
| **Process** | COM+ Application in `dllhost.exe` |
| **Location** | `src/BigDrive.Provider.Zip/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- Zip file access
- Read-only access to zip contents
- Virtual folder structure from zip entries

#### BigDrive.Provider.Sample.dll

| Property | Value |
|----------|-------|
| **Type** | ServicedComponent (COM+) |
| **Platform** | Any CPU |
| **Process** | COM+ Application in `dllhost.exe` |
| **Location** | `src/BigDrive.Provider.Sample/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- Reference implementation for provider developers
- Demonstrates all optional interfaces
- In-memory mock storage backend

---

### Shared Library Components

#### BigDrive.Interfaces.dll

| Property | Value |
|----------|-------|
| **Type** | Class Library |
| **Platform** | Any CPU |
| **Location** | `src/Interfaces/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- COM interface definitions for provider implementations
- Defines: IBigDriveEnumerate, IBigDriveFileData, IBigDriveFileInfo, IBigDriveFileOperations, IBigDriveAuthentication, IBigDriveRegistration
- Referenced by providers, Shell, and ShellFolder

#### BigDrive.Service.Interfaces.dll

| Property | Value |
|----------|-------|
| **Type** | Class Library |
| **Platform** | Any CPU |
| **Location** | `src/BigDrive.Service.Interfaces/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- COM interface definitions for BigDrive.Service
- Defines: IBigDriveProvision
- Referenced by Shell and Service (no direct project dependencies between them)

#### BigDrive.ConfigProvider.dll

| Property | Value |
|----------|-------|
| **Type** | Class Library |
| **Platform** | Any CPU |
| **Location** | `src/ConfigProvider/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- Registry read/write for drives and providers
- Abstracts registry access for other components
- Configuration hierarchy (drive-specific → provider default → hard-coded)

---

### Installation Components

#### BigDrive.Setup.exe

| Property | Value |
|----------|-------|
| **Type** | Console Application |
| **Platform** | Any CPU |
| **Location** | `src/BigDrive.Setup/` |
| **Framework** | .NET Framework 4.7.2 |

**Purpose:**
- COM+ application registration
- Service account creation (BigDriveInstaller)
- Provider installation
- Shell extension registration (via regsvr32)

---

## Execution Model Summary

### Out-of-Process COM+ Activation

```
┌──────────────────────┐
│ BigDrive.Shell.exe   │ ← User process (Any CPU)
│ or explorer.exe      │
└──────────┬───────────┘
           │
           │ CoCreateInstance(CLSID)
           │
           ▼
┌──────────────────────┐
│ dllhost.exe          │ ← COM+ Host Process
│                      │
│ Provider.Flickr.dll  │ ← Any CPU, JIT-compiled
│ (Interactive User)   │
└──────────────────────┘

┌──────────────────────┐
│ BigDrive.Shell.exe   │ ← User process (Any CPU)
└──────────┬───────────┘
           │
           │ CoCreateInstance(CLSID)
           │
           ▼
┌──────────────────────┐
│ dllhost.exe          │ ← COM+ Host Process
│                      │
│ BigDrive.Service.dll │ ← Any CPU, JIT-compiled
│ (BigDriveInstaller)  │ ← Elevated identity
└──────────────────────┘
```

### In-Process Loading

```
┌──────────────────────┐
│ explorer.exe         │ ← 32-bit or 64-bit
│                      │
│ ┌────────────────┐   │
│ │ BigDrive.      │   │ ← Must match Explorer bitness
│ │ ShellFolder.dll│   │    (x86 or x64)
│ └────────────────┘   │
└──────────────────────┘
```

---

## Why Any CPU for C# Components?

1. **Process Isolation**: Run in separate `dllhost.exe` processes via COM+
2. **No Bitness Dependency**: Don't integrate with architecture-specific shell APIs
3. **Cross-Architecture COM**: COM marshaling handles bitness differences across process boundaries
4. **Deployment Simplicity**: Single binary works on all architectures
5. **JIT Compilation**: .NET runtime compiles to native code at runtime based on host process

---

## See Also

- [Overview](overview.md) — High-level architecture
- [Build Configurations](build-configs.md) — How to build platform-specific vs Any CPU
- [Process Isolation](process-isolation.md) — COM+ activation details
- [Installation](installation.md) — Setup and registration
