# BigDrive.Service.Interfaces

## Overview

Defines the COM-visible interface contracts exposed by `BigDrive.Service` via COM+.
These interfaces are the contract layer between BigDrive.Shell (and other clients)
and the BigDrive.Service COM+ component.

## Architectural Role

```
BigDrive.Shell (client)              BigDrive.Service (COM+ dllhost.exe)
        │                            Identity: BigDriveInstaller
        │                                       │
        │  COM+ out-of-process activation       │
        ├──────────────────────────────────────►│
        │                                       │
        │  IBigDriveProvision.UnmountDrive()    │
        ├──────────────────────────────────────►│──► HKCR (COM class registration)
        │                                       │──► HKLM shell namespace
        │                                       │──► HKLM\SOFTWARE\BigDrive
        │                                       │──► Explorer refresh
        │◄──────────────────────────────────────┤
        │                                       │
  Both reference                          Implements
  BigDrive.Service.Interfaces             BigDrive.Service.Interfaces
```

The Shell must **never** reference `BigDrive.Service` directly or write to the
registry. All registry mutations go through these interfaces, which are implemented
by `BigDrive.Service` running in `dllhost.exe` under the **BigDriveInstaller**
identity — a local service account with write access to HKCR and HKLM.

This is distinct from **providers** (Flickr, Sample, Zip), which run in their own
COM+ applications as **Interactive User** (the logged-in user) to access the user's
Credential Manager and cloud APIs.

## Interface Definitions

### IBigDriveProvision (293D4995-FDFB-46FD-A0C6-A7DE2DA5B13F)

Drive provisioning and deprovisioning.

| Method | Purpose |
|--------|---------|
| `Create(driveGuid)` | Create a drive from existing registry config |
| `CreateFromConfiguration(json)` | Create a drive from JSON configuration |
| `UnmountDrive(driveGuid)` | Remove drive config, shell folder, refresh Explorer |

### IBigDriveConfiguration (D3F5A1B2-4C6E-4F8A-9D3E-1A2B3C4D5E6F)

Drive configuration retrieval.

| Method | Purpose |
|--------|---------|
| `GetConfiguration(guid)` | Return drive configuration as JSON |

### IBigDriveSetup (54B5E354-7982-4AC7-8D82-37C27E190113)

Setup validation.

| Method | Purpose |
|--------|---------|
| `Validate(activityId)` | Called by BigDrive.Setup to validate installation |

## Who References This Project

| Project | Why |
|---------|-----|
| `BigDrive.Service` | Implements these interfaces |
| `BigDrive.Shell` | Calls `IBigDriveProvision` via COM+ for mount/unmount |
| `BigDrive.Setup` | Calls `IBigDriveSetup` via COM+ for installation validation |

## Design Principle

This project exists so that clients (Shell, Setup) can reference the interface
types for COM+ casting without taking a dependency on `BigDrive.Service` itself.
This is distinct from `BigDrive.Interfaces`, which holds interfaces implemented
by **provider** COM+ components (Flickr, Sample, Zip, etc.).

## See Also

- [BigDrive.Interfaces README](../Interfaces/README.txt) — Provider interface definitions
- [Architecture Overview](../../docs/architecture/overview.md) — System architecture
