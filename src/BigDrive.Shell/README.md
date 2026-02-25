# BigDrive.Shell

A custom command-line shell for interacting with BigDrive virtual file systems.

## Overview

BigDrive.Shell provides a command-line interface for navigating and managing BigDrive
providers. It assigns drive letters (Z:, Y:, X:...) to registered BigDrives and
supports cross-drive file operations including mount/unmount of drives.

## User Documentation

For usage instructions, see the **[User Guide](../../docs/BigDrive.Shell.UserGuide.md)**.

## Debug Mode

Run the shell with `-d` or `--debug` to enable verbose tracing:

```
BigDrive.Shell.exe --debug
BigDrive.Shell.exe -d
```

Debug mode outputs:
- **COM calls**: Every call to provider interfaces with parameters
- **Path resolution**: How input paths are parsed and resolved
- **Method entry/exit**: Trace flow through command execution
- **HRESULT values**: Return codes from COM operations

Example output:
```
INFO [14:32:15.123] [COM] IBigDriveFileData.GetFileData(driveGuid=..., path="\A File.txt")
INFO [14:32:15.456] [COM] IBigDriveFileData.GetFileData returned S_OK: stream returned
```

---

## Shell Features

The shell provides familiar command-line editing features:

| Key | Action |
|-----|--------|
| **Tab** | Auto-complete commands, files, folders, drive letters |
| **Shift+Tab** | Cycle backwards through completions |
| **Up Arrow** | Previous command in history |
| **Down Arrow** | Next command in history |
| **F8** | Search history for commands starting with current input |
| **Left/Right** | Move cursor within line |
| **Home/End** | Jump to start/end of line |
| **Escape** | Clear current line |
| **Backspace/Delete** | Delete characters |

Command history:
- Stores up to 100 commands per session
- Duplicate consecutive commands are not stored
- Navigate with Up/Down arrows
- Current input is preserved when navigating
- **F8 prefix search**: Type partial command, press F8 to find matching history entries


---

## Project Structure

```
BigDrive.Shell/
├── Program.cs                 # Entry point, REPL loop
├── ShellContext.cs            # Session state (current drive, paths)
├── DriveLetterManager.cs      # Assigns drive letters, avoids OS conflicts
├── CommandProcessor.cs        # Parses input, dispatches to commands
├── ProviderFactory.cs         # COM+ provider activation
├── PathInfo.cs                # Path parsing utilities
├── ShellTrace.cs              # Debug tracing infrastructure (-d/--debug)
├── WildcardMatcher.cs         # Wildcard pattern matching (*, ?)
├── OAuthHelper.cs             # OAuth 2.0 authentication flows
├── OAuth1Helper.cs            # OAuth 1.0a authentication flows (for Flickr)
├── LineInput/                 # Console line input (Chain of Responsibility)
│   ├── IKeyHandler.cs         # Key handler interface
│   ├── LineBuffer.cs          # Buffer state and rendering
│   ├── ConsoleLineReader.cs   # Main reader, orchestrates chain
│   ├── CommandHistory.cs      # Shared history data (entries, navigation state)
│   ├── HistoryNavigationKeyHandler.cs  # Up/Down arrow history
│   ├── HistorySearchKeyHandler.cs      # F8 prefix search
│   ├── CompletionKeyHandler.cs # Tab completion
│   ├── NavigationKeyHandler.cs # Left/Right/Home/End
│   ├── EditingKeyHandler.cs   # Backspace/Delete/Escape
│   └── CharacterInputHandler.cs # Regular character input
├── Commands/
│   ├── ICommand.cs            # Command interface
│   ├── HelpCommand.cs
│   ├── ExitCommand.cs
│   ├── DrivesCommand.cs
│   ├── DirCommand.cs          # Supports wildcards
│   ├── CdCommand.cs
│   ├── CopyCommand.cs         # Supports wildcards
│   ├── MkdirCommand.cs
│   ├── DelCommand.cs          # Supports wildcards
│   ├── MountCommand.cs        # Create new drive (like 'net use')
│   ├── UnmountCommand.cs      # Remove drive
│   ├── SecretCommand.cs       # Manage secrets in Windows Credential Manager
│   ├── LoginCommand.cs        # OAuth authentication
│   ├── LogoutCommand.cs       # Clear authentication tokens
│   └── AuthStatusCommand.cs   # Check authentication status
└── Properties/
    └── AssemblyInfo.cs
```

## Wildcard Support

The shell supports DOS/PowerShell-style wildcards in file operations:

| Wildcard | Meaning | Example |
|----------|---------|---------|
| `*` | Match any characters (zero or more) | `*.txt`, `file*`, `*data*` |
| `?` | Match exactly one character | `file?.txt`, `doc??.pdf` |

**Supported commands:**
- `dir *.txt` - List files matching pattern
- `copy *.jpg c:\backup\` - Copy matching files
- `del *.tmp` - Delete matching files (with confirmation)

**Important:** Wildcard expansion happens in the shell, not the providers. The shell:
1. Enumerates all files from the provider
2. Filters by the wildcard pattern
3. Calls the provider once per matching file

---

## Architecture

> **CRITICAL DESIGN PRINCIPLE: Provider Isolation**
>
> The BigDrive Shell uses **out-of-process COM+ activation** (`CLSCTX_LOCAL_SERVER`) for
> all provider interactions. Provider assemblies and their dependencies are **NEVER loaded
> into the Shell process**. This prevents version conflicts and ensures stability.

### Key Components

| Component | Responsibility |
|-----------|----------------|
| `ShellContext` | Current drive letter, path per drive, session state |
| `DriveLetterManager` | Assigns drive letters to BigDrives, avoids OS drives |
| `CommandProcessor` | Parses input, dispatches to command handlers |
| `ProviderFactory` | Creates COM+ provider instances via **out-of-process** COM activation |
| `ICommand` | Interface implemented by all shell commands |

### Provider Agnosticism

The Shell is **completely provider-agnostic**. It:

- **Does NOT** contain any provider-specific code (no Flickr URLs, no OneDrive logic)
- **Does NOT** reference any provider assemblies or their dependencies
- **ONLY** communicates with providers through well-defined COM interfaces

This means you can add new providers without modifying or rebuilding the Shell.

### Registry Structure

BigDrive uses two registry locations under `HKLM\SOFTWARE\BigDrive`:

```
SOFTWARE\BigDrive\
├── Providers\                    ← Registered by regsvcs.exe (COM+ installation)
│   └── {PROVIDER-CLSID}\
│       ├── id   = "{CLSID}"
│       └── name = "Flickr Provider"
│
└── Drives\                       ← Created by 'mount' command or setup
    └── {DRIVE-GUID}\
        ├── id   = "{DRIVE-GUID}"
        ├── name = "My Flickr Photos"
        └── clsid = "{PROVIDER-CLSID}"   ← Points to provider
```

**Providers** are COM+ components registered during installation. They define *how* to
access a storage backend (Flickr API, Azure Blob, etc.).

**Drives** are user-created instances that use a provider. Multiple drives can use the
same provider (e.g., two different Flickr accounts).

### COM+ Provider Activation

The shell **never directly references provider assemblies**. Instead:

1. **Configuration** is read from the registry via `DriveManager.ReadConfigurations()`

2. **Provider activation** uses **out-of-process** COM+ with `CLSCTX_LOCAL_SERVER`:
   ```csharp
   // Get CLSID from drive configuration (registry)
   DriveConfiguration config = DriveManager.ReadConfiguration(driveGuid, token);

   // Activate provider via COM+ OUT-OF-PROCESS (runs in dllhost.exe)
   // This is critical - provider DLLs are NOT loaded into the Shell process
   Guid iidUnknown = new Guid("00000000-0000-0000-C000-000000000046");
   Guid clsid = config.CLSID;

   object provider;
   int hr = CoCreateInstance(ref clsid, null, CLSCTX_LOCAL_SERVER, ref iidUnknown, out provider);

   // Cast to BigDrive interface (COM proxy, not actual assembly)
   IBigDriveEnumerate enumerate = provider as IBigDriveEnumerate;
   ```

3. **Process isolation**: Providers run out-of-process in `dllhost.exe` (COM+ surrogate).
   The Shell process only holds COM interface proxies, not actual provider code.

### Why Out-of-Process Matters

| Benefit | Description |
|---------|-------------|
| **No Assembly Conflicts** | Provider dependencies (FlickrNet, Azure SDK, etc.) never enter the Shell process |
| **Version Independence** | Different providers can use different versions of the same library |
| **Stability** | Provider crashes don't crash the Shell |
| **Security** | Providers run in isolated COM+ application contexts |
| **Credential Access** | Providers can access Interactive User's Windows Credential Manager |

### Process Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│  BigDrive.Shell.exe (YOUR PROCESS - CLEAN)                              │
│                                                                         │
│  ┌──────────────────┐    ┌──────────────────┐    ┌──────────────────┐   │
│  │DriveLetterManager│───▶│ CommandProcessor │───▶│ ProviderFactory  │   │
│  │                  │    │                  │    │                  │   │
│  │ Z: → Drive A     │    │ dir, cd, copy... │    │ CoCreateInstance │   │
│  │ Y: → Drive B     │    │                  │    │CLSCTX_LOCAL_SERVER│  │
│  └──────────────────┘    └──────────────────┘    └────────┬─────────┘   │
│                                                           │             │
│  *** NO PROVIDER DLLS LOADED HERE ***                     │             │
│                                                           │             │
│  References (minimal):                                    │             │
│   - BigDrive.Interfaces (COM interface definitions only)  │             │
│   - BigDrive.ConfigProvider (registry access only)        │             │
│                                                           │             │
└───────────────────────────────────────────────────────────┼─────────────┘
                                                            │
                              COM+ IPC (out-of-process RPC/LRPC)
                                                            │
                                                            ▼
┌─────────────────────────────────────────────────────────────────────────┐
│  dllhost.exe (COM+ Surrogate - PROVIDER PROCESS)                        │
│                                                                         │
│  *** ALL PROVIDER CODE AND DEPENDENCIES LOADED HERE ***                 │
│                                                                         │
│  ┌─────────────────────┐  ┌─────────────────────┐                       │
│  │ Provider.Flickr     │  │ Provider.Sample     │  ...                  │
│  │                     │  │                     │                       │
│  │ - FlickrNet.dll     │  │ - No dependencies   │                       │
│  │ - System.Text.Json  │  │                     │                       │
│  │ - Other NuGet pkgs  │  │                     │                       │
│  │                     │  │                     │                       │
│  │ IBigDriveEnumerate  │  │ IBigDriveEnumerate  │                       │
│  │ IBigDriveFileData   │  │ IBigDriveFileData   │                       │
│  │ IBigDriveFileInfo   │  │ IBigDriveFileInfo   │                       │
│  │ IBigDriveAuthentication (optional)           │                       │
│  └─────────────────────┘  └─────────────────────┘                       │
│                                                                         │
│  Identity: Interactive User (can access Credential Manager)             │
└─────────────────────────────────────────────────────────────────────────┘
```

### Dependencies

| Assembly | Purpose |
|----------|---------|
| `BigDrive.Interfaces` | COM interface definitions (IBigDriveEnumerate, etc.) |
| `BigDrive.ConfigProvider` | DriveManager, ProviderManager for registry access |

The shell does **NOT** reference provider assemblies (Provider.Flickr, Provider.Sample, etc.)

### Interface Support

Providers implement a set of interfaces. Some are required, some are optional:

| Interface | Required | Description |
|-----------|----------|-------------|
| `IBigDriveEnumerate` | Yes | List folders and files |
| `IBigDriveFileInfo` | Yes | Get file/folder metadata |
| `IBigDriveFileOperations` | Yes | Create, delete, rename operations |
| `IBigDriveFileData` | Yes | Read/write file content |
| `IBigDriveAuthentication` | **No** | OAuth authentication |

**Handling Optional Interfaces:**

When the Shell calls a provider, it uses `as` casts to check interface support:

```csharp
IBigDriveAuthentication authProvider = provider as IBigDriveAuthentication;
if (authProvider == null)
{
    // Provider does NOT require authentication - this is normal!
    // The Sample provider, for example, doesn't implement this interface.
}
```

Providers that don't require authentication (like the Sample provider) simply don't implement
`IBigDriveAuthentication`. The Shell handles this gracefully.

---

## OAuth Authentication

BigDrive.Shell includes generic OAuth support for authenticating with cloud providers.
The implementation follows patterns from Azure CLI, GitHub CLI, and other modern CLIs
(see [OAuth Authentication Patterns](../../research/OAuth%20Authentication%20Patterns%20in%20Windows%20CLI%20Applications.md)).

### Supported OAuth Flows

| Flow | Use Case |
|------|----------|
| **Authorization Code with Loopback** | Default. Opens browser, listens on localhost for redirect. Seamless UX. |
| **Device Code** | For headless/SSH. Displays URL and code. User authorizes on any device. |
| **OAuth 1.0a** | For legacy providers like Flickr that don't support OAuth 2.0. |

### Authentication Commands

| Command | Description |
|---------|-------------|
| `login` | Authenticate with the current drive's provider (browser flow) |
| `login --device-code` | Use device code flow (for SSH/headless) |
| `logout` | Clear cached OAuth tokens |
| `logout --all` | Clear all secrets including API keys |
| `authstatus` | Check current authentication status |

### Token Storage

Tokens are stored securely using **Windows Credential Manager** (per-user, encrypted).
This follows the same pattern as GitHub CLI and Azure CLI. Tokens are associated with
each drive's GUID, allowing multiple accounts per provider.

### Provider Authentication Interface

Providers can implement `IBigDriveAuthentication` to describe their OAuth requirements:

```csharp
public interface IBigDriveAuthentication
{
    int GetAuthenticationInfo(Guid driveGuid, out AuthenticationInfo authInfo);
    int OnAuthenticationComplete(Guid driveGuid, string accessToken, string refreshToken, int expiresIn);
    int IsAuthenticated(Guid driveGuid, out bool isAuthenticated);
}
```

The shell handles the OAuth flow generically; the provider just describes its endpoints.

---

## See Also

- [User Guide](../../docs/BigDrive.Shell.UserGuide.md) — Complete command reference
- [Provider Development Guide](../../docs/ProviderDevelopmentGuide.md) — Creating new providers
- [BigDrive.Interfaces](../Interfaces/README.txt) — Interface definitions
- [BigDrive.ConfigProvider](../ConfigProvider/README.txt) — Registry configuration

---

*Copyright © Wayne Walter Berry. All rights reserved.*
