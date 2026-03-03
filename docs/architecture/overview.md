# BigDrive Architecture Overview

## What is BigDrive?

BigDrive exposes cloud storage services (Flickr, Azure Blob, etc.) as virtual file systems in Windows. Users can browse and manage remote files using familiar tools like Windows Explorer or BigDrive.Shell (command-line interface).

---

## Table of Contents

This overview provides a high-level introduction to BigDrive's architecture. For detailed information, see:

- **[Components](components.md)** — Detailed breakdown of C++ and C# components, platform architectures, and execution models
- **[Build Configurations](build-configs.md)** — Platform-specific vs Any CPU projects, solution configurations, and toolset versions
- **[Registry Structure](registry.md)** — Registry layout, providers vs drives, and configuration hierarchy
- **[Interface Hierarchy](interfaces.md)** — COM interface definitions, provider contracts, and service interfaces
- **[Data Flow](data-flow.md)** — Request/response flows for common operations (dir, copy, mount)
- **[Process Isolation](process-isolation.md)** — COM+ activation, dllhost.exe processes, and security boundaries
- **[Installation](installation.md)** — Setup process, COM+ registration, and shell integration (existing file)

---

## High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              USER INTERFACES                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────┐         ┌───────────────────────┐                │
│  │   Windows Explorer    │         │   BigDrive.Shell      │                │
│  │                       │         │                       │                │
│  │  BigDrive.ShellFolder │         │  Command-line shell   │                │
│  │  (Shell Extension)    │         │  dir, cd, copy, mount │                │
│  └───────────┬───────────┘         └────┬─────────────┬────┘                │
│              │                          │             │                     │
│              │ IShellFolder             │ IBigDrive*  │ IBigDriveProvision  │
│              │ IEnumIDList              │ (providers) │ (service)           │
│              │                          │             │                     │
│              └──────────┬───────────────┘             │                     │
│                         │                             │                     │
│           CoCreateInstance(CLSID)        CoCreateInstance(CLSID)           │
│                         │                             │                     │
└─────────────────────────┼─────────────────────────────┼─────────────────────┘
                          │                             │
              COM+ Activation               COM+ Activation
              (out-of-process)              (out-of-process)
                          │                             │
                          ▼                             ▼
┌──────────────────────────────────┐ ┌────────────────────────────────┐
│  COM+ APPLICATION (dllhost.exe)  │ │  COM+ APPLICATION (dllhost.exe)│
│  Providers                       │ │  BigDrive.Service              │
├──────────────────────────────────┤ ├────────────────────────────────┤
│                                  │ │                                │
│ ┌────────────────────┐           │ │  IBigDriveProvision            │
│ │ Provider.Flickr    │  ...      │ │  ├── Create()                  │
│ │ IBigDriveEnumerate │           │ │  ├── CreateFromConfiguration() │
│ │ IBigDriveFileData  │           │ │  └── UnmountDrive()            │
│ └────────┬───────────┘           │ │                                │
│          │                       │ │  Writes to HKCR, HKLM shell   │
│ Identity: Interactive User       │ │  namespace, and HKLM\BigDrive. │
└──────────┼───────────────────────┘ │  Refreshes Explorer.           │
           │                         │                                │
           ▼                         │ Identity: BigDriveInstaller    │
┌─────────────────────┐              │ (elevated local service acct)  │
│   Flickr API        │              └────────────────────────────────┘
│   Flickr API        │
│   (HTTPS)           │  ...other cloud APIs
└─────────────────────┘
```

---

## Key Architectural Principles

### 1. Out-of-Process COM+ Activation

All providers and services run in separate `dllhost.exe` processes via COM+, ensuring:
- **Process isolation** — Provider crashes don't affect Explorer or the Shell
- **Security boundaries** — Providers run as Interactive User; Service runs as elevated BigDriveInstaller
- **Cross-architecture support** — C# components (Any CPU) can be called from C++ components (x86/x64)

### 2. Provider Abstraction

Providers are pluggable COM+ components that implement standard interfaces (`IBigDriveEnumerate`, `IBigDriveFileData`, etc.). New storage backends can be added without modifying core BigDrive components.

### 3. Service and Provider Independence

BigDrive.Service and the providers are **completely independent** of each other:
- Providers must **not** reference `BigDrive.Service.dll` — they only reference
  `BigDrive.Interfaces` and `BigDrive.ConfigProvider`
- BigDrive.Service must **not** reference any provider assembly
- All cross-component communication uses COM+ out-of-process activation
  (`CoCreateInstance`), never direct assembly references

This isolation ensures each component can be built, deployed, and updated
independently without affecting the others. See [Components — Dependency Rules](components.md#dependency-rules)
for the full reference graph.

### 4. Drive Instances

A single provider (e.g., Flickr) can support multiple drive instances with different configurations (personal account, work account). Each drive has its own GUID and registry configuration.

### 5. Platform-Specific Shell Extensions

C++ shell extensions (`BigDrive.ShellFolder.dll`) must be built for both x86 and x64 to support 32-bit and 64-bit Windows Explorer processes. C# components remain "Any CPU" and run out-of-process.

---

## Core Concepts

### Providers vs Drives

| Concept | Description | Example |
|---------|-------------|---------|
| **Provider** | COM+ component that knows *how* to access a storage backend | "Flickr Provider" |
| **Drive** | User-created instance using a provider to access *specific* storage | "My Personal Flickr" (drive Z:) |

**Registry Locations:**
- Providers: `HKLM\SOFTWARE\BigDrive\Providers\{CLSID}`
- Drives: `HKLM\SOFTWARE\BigDrive\Drives\{GUID}`

See [Registry Structure](registry.md) for detailed registry layout.

---

## Component Roles

| Component | Purpose | Process | Platform |
|-----------|---------|---------|----------|
| **BigDrive.Shell.exe** | Command-line interface (dir, cd, copy, mount) | Standalone .NET | Any CPU |
| **BigDrive.ShellFolder.dll** | Windows Explorer integration (namespace extension) | Loads into explorer.exe | x86, x64 |
| **BigDrive.Service.dll** | Drive provisioning with elevated registry access | COM+ (dllhost.exe) | Any CPU |
| **Provider.*.dll** | Storage backend implementations (Flickr, Archive, Zip) | COM+ (dllhost.exe) | Any CPU |

See [Components](components.md) for complete component breakdown.

---

## Quick Start References

- **Users**: See [BigDrive.Shell User Guide](../BigDrive.Shell.UserGuide.md) for command usage
- **Provider Developers**: See [Provider Development Guide](../provider-development/README.md) for creating new providers
- **Contributors**: Read individual project READMEs (e.g., `src/BigDrive.Shell/README.md`) before making changes

---

## See Also

- [Components Documentation](components.md)
- [Build Configurations](build-configs.md)
- [Registry Structure](registry.md)
- [Interface Hierarchy](interfaces.md)
- [Installation Flow](installation.md)
