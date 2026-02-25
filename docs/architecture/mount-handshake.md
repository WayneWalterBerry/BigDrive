# Mount Command Handshake Protocol

## Overview

This document describes the complete handshake protocol that occurs when a user executes
the `mount` command in BigDrive.Shell to create a new drive. The protocol involves
communication between:

1. **BigDrive.Shell** (user-facing command processor)
2. **Provider COM+ Application** (storage backend, runs in dllhost.exe)
3. **Windows Registry** (persistent configuration storage)

---

## High-Level Flow

```
User types: mount
     ↓
Shell lists available providers
     ↓
User selects provider & enters drive name
     ↓
Shell queries provider for required parameters (IBigDriveDriveInfo)
     ↓
User enters parameter values
     ↓
Shell writes drive configuration to registry (DriveManager)
     ↓
Drive is now accessible at assigned drive letter
```

---

## Complete Handshake Sequence

### Phase 1: Provider Discovery

**Actor**: BigDrive.Shell

```
┌──────────────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                                   │
│                                                                      │
│ MountCommand.Execute()                                               │
│   ↓                                                                  │
│ ProviderManager.ReadProviders()                                      │
│   → Reads HKLM\SOFTWARE\BigDrive\Providers\*                        │
│   → Returns list of ProviderConfiguration objects                    │
│                                                                      │
│ Each ProviderConfiguration contains:                                 │
│   • Id (CLSID)           → {C7A1B2D3-E4F5-6789-AB01-CD23EF456789}   │
│   • Name                 → "Zip"                                     │
└──────────────────────────────────────────────────────────────────────┘
```

**Registry Read**:
```
HKLM\SOFTWARE\BigDrive\Providers\
  └── {C7A1B2D3-E4F5-6789-AB01-CD23EF456789}\
      ├── id   = "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}"
      └── name = "Zip"
```

**Output to User**:
```
Available providers:

  [1] BigDrive.Provider.Sample
      CLSID: f8fe2e5a-e8b8-4207-bc04-ea4bcd4c4361

  [2] Zip
      CLSID: c7a1b2d3-e4f5-6789-ab01-cd23ef456789

  [3] Flickr Photos
      CLSID: b3d8f2a1-7c4e-4a9b-8e1f-2c3d4e5f6a7b

Select provider number:
```

---

### Phase 2: Basic Drive Information

**Actor**: User

User input:
```
Select provider number: 2
Enter drive name: MyZipDrive
```

**Actor**: BigDrive.Shell

```
┌──────────────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                                   │
│                                                                      │
│ CreateDrive()                                                        │
│   ↓                                                                  │
│ Generate new Guid                                                    │
│   → driveGuid = {74BC469C-34D2-43B2-B263-232E07273782}              │
│                                                                      │
│ Create DriveConfiguration:                                           │
│   • Id    = {74BC469C-34D2-43B2-B263-232E07273782}                  │
│   • Name  = "MyZipDrive"                                             │
│   • CLSID = {C7A1B2D3-E4F5-6789-AB01-CD23EF456789}                  │
│   • Properties = {} (empty dictionary, to be populated)              │
└──────────────────────────────────────────────────────────────────────┘
```

---

### Phase 3: Provider Parameter Discovery

**Actor**: BigDrive.Shell

The shell now needs to know what **provider-specific parameters** are required for this
type of drive (e.g., Zip needs "ZipFilePath", Flickr needs "ApiKey", etc.).

```
┌──────────────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                                   │
│                                                                      │
│ QueryDriveParameters(providerClsid)                                  │
│   ↓                                                                  │
│ ProviderFactory.GetDriveInfoProvider(providerClsid)                  │
│   ↓                                                                  │
│ CoCreateInstance(                                                    │
│   CLSID = {C7A1B2D3-E4F5-6789-AB01-CD23EF456789},                   │
│   Context = CLSCTX_LOCAL_SERVER                                      │
│ )                                                                    │
│   ↓                                                                  │
│ Cast to IBigDriveDriveInfo                                           │
└────────────────────────────┬─────────────────────────────────────────┘
                             │
                   COM+ Activation
                             │
                             ▼
┌──────────────────────────────────────────────────────────────────────┐
│ dllhost.exe (COM+ Surrogate)                                         │
│                                                                      │
│ BigDrive.Provider.Zip.dll loaded                                     │
│   ↓                                                                  │
│ Provider instance created                                            │
│   ↓                                                                  │
│ Returns IBigDriveDriveInfo interface pointer                         │
└────────────────────────────┬─────────────────────────────────────────┘
                             │
                   COM Interface Call
                             │
                             ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Shell calls:                                                         │
│                                                                      │
│ string json = driveInfo.GetDriveParameters()                         │
│                                                                      │
│ Provider returns JSON:                                               │
│ [                                                                    │
│   {                                                                  │
│     "name": "ZipFilePath",                                           │
│     "description": "Full path to the ZIP file (existing or new).",   │
│     "type": "filepath"                                               │
│   }                                                                  │
│ ]                                                                    │
└──────────────────────────────────────────────────────────────────────┘
```

**Data Transferred**:

**Direction**: Provider → Shell  
**Format**: JSON string  
**Schema**: Array of `DriveParameterDefinition` objects

```json
[
  {
    "name": "ZipFilePath",
    "description": "Full path to the ZIP file (existing or new).",
    "type": "filepath"
  }
]
```

**DriveParameterDefinition Fields**:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `name` | string | Yes | Registry value name for this parameter (e.g., "ZipFilePath") |
| `description` | string | No | User-facing prompt text |
| `type` | string | No | Input type (see below for supported types) |

**Supported Parameter Types**:

| Type | Tab Completion | Validation | Use Case |
|------|----------------|------------|----------|
| `"string"` | No | None | Plain text (default if type omitted) |
| `"existing-file"` | Yes | Must exist | Configuration files, certificates, existing archives |
| `"filepath"` | Yes | None | New or existing files (e.g., new ZIP to create) |

**Legacy Support**: `"file"` is treated as `"filepath"` for backward compatibility.

---

### Phase 4: User Input Collection

**Actor**: BigDrive.Shell

The shell now prompts the user for each parameter defined by the provider.

```
┌──────────────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                                   │
│                                                                      │
│ For each parameter in JSON array:                                    │
│   ↓                                                                  │
│ Display description (if provided)                                    │
│ Display prompt: "{name}: "                                           │
│   ↓                                                                  │
│ If type == "file":                                                   │
│   → Enable Tab key file-path completion                              │
│ Else:                                                                │
│   → Standard string input                                            │
│   ↓                                                                  │
│ Store value in Properties dictionary                                 │
└──────────────────────────────────────────────────────────────────────┘
```

**Output to User**:
```
This provider requires the following parameters:

  Full path to the ZIP file (existing or new).
  ZipFilePath: C:\MyArchive.zip

```

**Internal State** (in memory):
```csharp
DriveConfiguration driveConfig = new DriveConfiguration
{
    Id = {74BC469C-34D2-43B2-B263-232E07273782},
    Name = "MyZipDrive",
    CLSID = {C7A1B2D3-E4F5-6789-AB01-CD23EF456789},
    Properties = new Dictionary<string, string>
    {
        { "ZipFilePath", @"C:\MyArchive.zip" }
    }
};
```

---

### Phase 5: Registry Persistence

**Actor**: BigDrive.Shell

Once all parameters are collected, the shell writes the complete drive configuration
to the Windows registry.

```
┌──────────────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                                   │
│                                                                      │
│ DriveManager.WriteConfiguration(driveConfig)                         │
│   ↓                                                                  │
│ Open/Create Registry Key:                                            │
│   HKLM\SOFTWARE\BigDrive\Drives\{74BC469C-...}                       │
│   ↓                                                                  │
│ Write standard properties (using reflection):                        │
│   • SetValue("id",    "{74BC469C-...}")                              │
│   • SetValue("name",  "MyZipDrive")                                  │
│   • SetValue("clsid", "{C7A1B2D3-...}")                              │
│   ↓                                                                  │
│ Write custom properties (from Properties dictionary):                │
│   • SetValue("ZipFilePath", "C:\MyArchive.zip")                      │
└──────────────────────────────────────────────────────────────────────┘
```

**Registry Write**:
```
HKLM\SOFTWARE\BigDrive\Drives\{74BC469C-34D2-43B2-B263-232E07273782}\
  ├── id          = "{74BC469C-34D2-43B2-B263-232E07273782}"
  ├── name        = "MyZipDrive"
  ├── clsid       = "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}"
  └── ZipFilePath = "C:\MyArchive.zip"
```

---

### Phase 6: Drive Letter Assignment

**Actor**: BigDrive.Shell

After the registry is written, the shell refreshes its drive list and assigns a drive letter.

```
┌──────────────────────────────────────────────────────────────────────┐
│ BigDrive.Shell.exe                                                   │
│                                                                      │
│ context.RefreshDrives()                                              │
│   ↓                                                                  │
│ DriveLetterManager.Refresh()                                         │
│   → Reads HKLM\SOFTWARE\BigDrive\Drives\*                            │
│   → Assigns next available letter (Y:, Z:, etc.)                     │
│   ↓                                                                  │
│ context.DriveLetterManager.GetDriveLetter(driveGuid)                 │
│   → Returns 'Y'                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

**Output to User**:
```
Drive mounted successfully!

  Name:     MyZipDrive
  GUID:     74bc469c-34d2-43b2-b263-232e07273782
  Provider: Zip

Use 'cd Y:' to access the new drive.
```

---

## Data Structure Reference

### ProviderConfiguration

Stored in: `HKLM\SOFTWARE\BigDrive\Providers\{CLSID}\`

| Property | Type | Description |
|----------|------|-------------|
| `Id` | Guid | Provider CLSID (COM class identifier) |
| `Name` | string | Display name (e.g., "Zip", "Flickr Provider") |

**Example**:
```csharp
new ProviderConfiguration
{
    Id = new Guid("C7A1B2D3-E4F5-6789-AB01-CD23EF456789"),
    Name = "Zip"
}
```

---

### DriveConfiguration

Stored in: `HKLM\SOFTWARE\BigDrive\Drives\{GUID}\`

| Property | Type | Description |
|----------|------|-------------|
| `Id` | Guid | Unique drive identifier |
| `Name` | string | User-specified drive name |
| `CLSID` | Guid | Provider CLSID this drive uses |
| `Properties` | Dictionary<string,string> | Provider-specific parameters (e.g., "ZipFilePath") |

**Example**:
```csharp
new DriveConfiguration
{
    Id = new Guid("74BC469C-34D2-43B2-B263-232E07273782"),
    Name = "MyZipDrive",
    CLSID = new Guid("C7A1B2D3-E4F5-6789-AB01-CD23EF456789"),
    Properties = new Dictionary<string, string>
    {
        { "ZipFilePath", @"C:\MyArchive.zip" }
    }
}
```

---

### DriveParameterDefinition

Returned by: `IBigDriveDriveInfo.GetDriveParameters()`  
Format: JSON array serialized to string

| Property | Type | Required | Description |
|----------|------|----------|-------------|
| `name` | string | Yes | Registry key for this parameter |
| `description` | string | No | User-facing prompt text |
| `type` | string | No | Input type: `"string"` (default) or `"file"` |

**Example JSON**:
```json
[
  {
    "name": "ZipFilePath",
    "description": "Full path to the ZIP file.",
    "type": "file"
  }
]
```

**Provider Implementation** (Provider.IBigDriveDriveInfo.cs):
```csharp
public string GetDriveParameters()
{
    DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
    {
        new DriveParameterDefinition
        {
            Name = "ZipFilePath",
            Description = "Full path to the ZIP file (existing or new).",
            Type = "filepath"
        }
    };

    return JsonSerializer.Serialize(parameters);
}
```

---

## Detailed Sequence Diagram

```
User               Shell               ProviderFactory        Registry           Provider (dllhost.exe)
 │                   │                       │                   │                       │
 │─mount─────────────►                       │                   │                       │
 │                   │                       │                   │                       │
 │                   │─ReadProviders()───────────────────────────►                       │
 │                   │                       │                   │                       │
 │                   │◄────List of providers with CLSID──────────│                       │
 │                   │                       │                   │                       │
 │◄─Show provider───│                       │                   │                       │
 │  list             │                       │                   │                       │
 │                   │                       │                   │                       │
 │─Select [2] Zip────►                       │                   │                       │
 │                   │                       │                   │                       │
 │─Enter "MyZip"─────►                       │                   │                       │
 │                   │                       │                   │                       │
 │                   │─GetDriveInfoProvider()─►                  │                       │
 │                   │  (providerClsid)      │                   │                       │
 │                   │                       │                   │                       │
 │                   │                       │─CoCreateInstance──────────────────────────►
 │                   │                       │ (CLSID, CLSCTX_   │                       │
 │                   │                       │  LOCAL_SERVER)    │                       │
 │                   │                       │                   │                       │
 │                   │                       │◄─────Provider instance (IBigDriveDriveInfo)
 │                   │                       │                   │                       │
 │                   │◄─IBigDriveDriveInfo───│                   │                       │
 │                   │                       │                   │                       │
 │                   │─GetDriveParameters()──────────────────────────────────────────────►
 │                   │                       │                   │                       │
 │                   │◄─────JSON array of parameter definitions────────────────────────────
 │                   │  [{"name":"ZipFilePath","description":"Full path...","type":"filepath"}]
 │                   │                       │                   │                       │
 │◄─Prompt for param│                       │                   │                       │
 │  "ZipFilePath"    │                       │                   │                       │
 │                   │                       │                   │                       │
 │─Enter path────────►                       │                   │                       │
 │  "C:\Archive.zip" │                       │                   │                       │
 │                   │                       │                   │                       │
 │                   │─WriteConfiguration()──────────────────────►                       │
 │                   │  (DriveConfiguration) │                   │                       │
 │                   │                       │                   │                       │
 │                   │                       │      Registry writes:                     │
 │                   │                       │      ├─ id                                │
 │                   │                       │      ├─ name                              │
 │                   │                       │      ├─ clsid                             │
 │                   │                       │      └─ ZipFilePath                       │
 │                   │                       │                   │                       │
 │                   │◄─────Success──────────────────────────────│                       │
 │                   │                       │                   │                       │
 │                   │─RefreshDrives()───────────────────────────►                       │
 │                   │                       │                   │                       │
 │                   │◄─────Drive list with letter assignments───│                       │
 │                   │       (Y: → {74BC469C-...})                │                       │
 │                   │                       │                   │                       │
 │◄─"Drive mounted   │                       │                   │                       │
 │   Use 'cd Y:'"    │                       │                   │                       │
```

---

## Phase-by-Phase Data Flow

### Phase 1: Provider Discovery

**Request**: None (reads from registry only)

**Response**: List of `ProviderConfiguration`

```csharp
List<ProviderConfiguration> providers = [
    new ProviderConfiguration 
    { 
        Id = new Guid("C7A1B2D3-..."), 
        Name = "Zip" 
    },
    // ... other providers
];
```

---

### Phase 2: User Input

**Input from user**:
- Provider selection: `2`
- Drive name: `MyZipDrive`

**Internal processing**:
```csharp
ProviderConfiguration selectedProvider = providers[1]; // index 1 = provider #2
Guid driveId = Guid.NewGuid(); // {74BC469C-...}
```

---

### Phase 3: Parameter Discovery (COM Call)

**Step 3.1**: CoCreateInstance

**Direction**: Shell → COM Runtime → Provider  
**Method**: `CoCreateInstance()`  
**Parameters**:
- `rclsid`: `{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}`
- `dwClsContext`: `CLSCTX_LOCAL_SERVER` (0x4) — out-of-process activation
- `riid`: `IID_IUnknown` (`{00000000-0000-0000-C000-000000000046}`)

**Result**: Provider COM object instance running in dllhost.exe

---

**Step 3.2**: QueryInterface to IBigDriveDriveInfo

**Direction**: Shell → Provider  
**Method**: `QueryInterface(IID_IBigDriveDriveInfo)`  
**Result**: `IBigDriveDriveInfo` interface pointer, or null if not implemented

---

**Step 3.3**: GetDriveParameters() COM Call

**Direction**: Shell → Provider  
**Method**: `IBigDriveDriveInfo.GetDriveParameters()`  
**Parameters**: None  
**Return**: JSON string

**Provider Code Executed** (Provider.IBigDriveDriveInfo.cs):
```csharp
public string GetDriveParameters()
{
    DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
    {
        new DriveParameterDefinition
        {
            Name = "ZipFilePath",
            Description = "Full path to the ZIP file (existing or new).",
            Type = "filepath"
        }
    };

    return JsonSerializer.Serialize(parameters);
}
```

**Returned JSON**:
```json
[{"name":"ZipFilePath","description":"Full path to the ZIP file (existing or new).","type":"filepath"}]
```

---

### Phase 4: User Parameter Input

**Shell Processing**:
```csharp
Dictionary<string, string> properties = new Dictionary<string, string>();

JsonElement[] parameters = JsonSerializer.Deserialize<JsonElement[]>(json);

foreach (JsonElement param in parameters)
{
    string name = param.GetProperty("name").GetString();        // "ZipFilePath"
    string description = param.GetProperty("description").GetString(); // "Full path..."
    string type = param.GetProperty("type").GetString();        // "filepath"

    Console.WriteLine("  {0}", description);
    Console.Write("  {0}: ", name);

    string value;
    if (type == "existing-file")
    {
        value = ReadLineWithFileCompletion();  // Tab completion
        if (!File.Exists(value))
        {
            Console.WriteLine("  Error: File does not exist: {0}", value);
            return null;  // Cancel mount
        }
    }
    else if (type == "filepath" || type == "file")  // "file" = legacy
    {
        value = ReadLineWithFileCompletion();  // Tab completion, no validation
    }
    else  // "string"
    {
        value = Console.ReadLine();            // Standard input
    }

    properties[name] = value.Trim();    // "C:\MyArchive.zip" (may not exist yet)
}

driveConfig.Properties = properties;
```

---

### Phase 5: Registry Write

**Direction**: Shell → Windows Registry  
**Method**: `DriveManager.WriteConfiguration()`  
**Parameters**: `DriveConfiguration` object

**Registry Operations**:
```
CreateSubKey: HKLM\SOFTWARE\BigDrive\Drives\{74BC469C-34D2-43B2-B263-232E07273782}
  ↓
SetValue("id",    "{74BC469C-34D2-43B2-B263-232E07273782}")
SetValue("name",  "MyZipDrive")
SetValue("clsid", "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}")
  ↓
SetValue("ZipFilePath", "C:\MyArchive.zip")
```

**Final Registry State**:
```
HKLM\SOFTWARE\BigDrive\Drives\{74BC469C-34D2-43B2-B263-232E07273782}\
  ├── id          (REG_SZ) = "{74BC469C-34D2-43B2-B263-232E07273782}"
  ├── name        (REG_SZ) = "MyZipDrive"
  ├── clsid       (REG_SZ) = "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}"
  └── ZipFilePath (REG_SZ) = "C:\MyArchive.zip"
```

---

### Phase 6: Drive Letter Assignment

**Direction**: Shell internal operation  
**Method**: `DriveLetterManager.Refresh()` → `GetDriveLetter(driveGuid)`

**Algorithm**:
1. Read all drives from registry
2. Assign letters starting from Z: and working backwards (Z, Y, X, ...)
3. Skip letters already in use by OS drives

**Result**: Drive `{74BC469C-...}` is assigned letter `Y:`

---

## Provider Interface Requirements for Mount

### Required Interface: IBigDriveDriveInfo (Optional)

**When to implement**: If your provider requires custom configuration parameters
(API keys, file paths, endpoint URLs, etc.)

**Interface Definition**:
```csharp
[ComVisible(true)]
[Guid("3A2B1C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D")]
public interface IBigDriveDriveInfo
{
    string GetDriveParameters();
}
```

**Return Value**: JSON string containing array of `DriveParameterDefinition` objects

**If not implemented**: Shell assumes the provider requires **no custom parameters**
and creates the drive with only the standard properties (id, name, clsid).

---

### Example: Provider That Requires No Parameters

**Provider**: Sample provider (in-memory tree structure)

**Implementation**: Does **not** implement `IBigDriveDriveInfo`

**Mount behavior**: User only needs to provide a drive name

```
BD> mount

Available providers:

  [1] BigDrive.Provider.Sample
      CLSID: f8fe2e5a-e8b8-4207-bc04-ea4bcd4c4361

Select provider number: 1
Enter drive name: TestDrive

Drive mounted successfully!  ← No parameter prompts!
```

---

### Example: Provider With Multiple Parameters

**Provider**: Hypothetical Azure Blob provider

**Implementation** (Provider.IBigDriveDriveInfo.cs):
```csharp
public string GetDriveParameters()
{
    DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
    {
        new DriveParameterDefinition
        {
            Name = "StorageAccountName",
            Description = "Azure Storage account name",
            Type = "string"
        },
        new DriveParameterDefinition
        {
            Name = "ContainerName",
            Description = "Blob container name",
            Type = "string"
        },
        new DriveParameterDefinition
        {
            Name = "CertificatePath",
            Description = "Path to existing authentication certificate",
            Type = "existing-file"
        },
        new DriveParameterDefinition
        {
            Name = "LocalCachePath",
            Description = "Local cache file path (will be created)",
            Type = "filepath"
        }
    };

    return JsonSerializer.Serialize(parameters);
}
```

**Mount interaction**:
```
Select provider number: 3
Enter drive name: MyAzureBlobs

This provider requires the following parameters:

  Azure Storage account name
  StorageAccountName: mystorageacct

  Blob container name
  ContainerName: documents

  Path to existing authentication certificate
  CertificatePath: C:\Certs\azure.pfx  ← Must exist or mount fails

  Local cache file path (will be created)
  LocalCachePath: C:\Temp\azurecache.db  ← Can be new, no validation

Drive mounted successfully!
```

---

## Parameter Type Behaviors

### Type: "string"

- Standard `Console.ReadLine()` input
- No Tab completion
- No validation
- Use for: text values, URLs, account names, numeric IDs

---

### Type: "existing-file"

- Enables **Tab key completion** for local file paths
- Shell uses `ReadLineWithFileCompletion()` internally
- **Validates that file exists** after user input
- Returns error if path doesn't exist
- Pressing Tab cycles through matching existing files/directories
- Use for: configuration files that must exist, certificates, source data files

**Example interaction**:
```
  ConfigFile: C:\Users\wayne\[TAB]
  ConfigFile: C:\Users\wayne\Documents\[TAB]
  ConfigFile: C:\Users\wayne\Documents\config.xml
```

If file doesn't exist:
```
  ConfigFile: C:\NonExistent\file.xml

  Error: File does not exist: C:\NonExistent\file.xml
Mount cancelled.
```

---

### Type: "filepath"

- Enables **Tab key completion** for local file paths
- Shell uses `ReadLineWithFileCompletion()` internally
- **No validation** — allows non-existent paths
- Pressing Tab cycles through matching existing files/directories (completion only shows existing)
- Use for: new files to create (ZIP archives, databases), output paths, log files

**Example interaction (existing file)**:
```
  ZipFilePath: C:\Users\wayne\[TAB]
  ZipFilePath: C:\Users\wayne\Documents\[TAB]
  ZipFilePath: C:\Users\wayne\Documents\Archive.zip
```

**Example interaction (new file)**:
```
  ZipFilePath: C:\NewArchives\MyNewArchive.zip  ← Path doesn't exist yet

Drive mounted successfully!  ← Shell accepts it
```

**Provider responsibility**: Provider must handle the non-existent path appropriately:
- Create the file if needed (e.g., ZipClientWrapper creates empty ZIP)
- Create parent directories if needed
- Return appropriate errors if path is invalid

---

### Type: "file" (Legacy)

- Treated as **"filepath"** (existing or new files allowed)
- Maintained for backward compatibility with existing providers
- New providers should use `"existing-file"` or `"filepath"` explicitly

---

## Runtime: How Providers Access Their Configuration

After a drive is mounted, providers receive the `driveGuid` in every interface method call.
They can read their configuration using `DriveManager`:

```csharp
// In provider implementation (e.g., Provider.IBigDriveEnumerate.cs)
public string[] EnumerateFiles(Guid driveGuid, string path)
{
    // Read the ZipFilePath parameter that was stored during mount
    string zipFilePath = DriveManager.ReadDriveProperty(
        driveGuid, 
        "ZipFilePath", 
        CancellationToken.None
    );
    
    // Use the path to access the ZIP file
    using (ZipArchive archive = ZipFile.OpenRead(zipFilePath))
    {
        // ... enumerate files
    }
}
```

**What happens under the hood**:
```csharp
DriveManager.ReadDriveProperty(driveGuid, "ZipFilePath", ...)
  ↓
Opens: HKLM\SOFTWARE\BigDrive\Drives\{driveGuid}\
  ↓
GetValue("ZipFilePath")
  ↓
Returns: "C:\MyArchive.zip"
```

---

## Error Handling

### Provider Not Found

**Condition**: User enters invalid provider number or name

**Shell behavior**:
```
Provider not found: 99
Use 'mount' without arguments to see available providers.
```

---

### Provider Does Not Implement IBigDriveDriveInfo

**Condition**: `GetDriveInfoProvider()` returns null

**Shell behavior**: Assumes no custom parameters required, skips parameter prompt phase

---

### Provider Returns Invalid JSON

**Condition**: `GetDriveParameters()` returns malformed JSON

**Shell behavior**:
```
Warning: Provider returned invalid parameter definitions.
Drive mounted successfully!  ← Continues with no custom parameters
```

---

### Registry Access Denied

**Condition**: User lacks write permissions to `HKLM\SOFTWARE\BigDrive\Drives`

**Shell behavior**:
```
Access denied. Run BigDrive.Shell as Administrator to mount drives.
```

**Solution**: Either:
1. Run BigDrive.Shell as Administrator, or
2. Grant "Users" group write access to `HKLM\SOFTWARE\BigDrive\Drives`

---

## COM+ Activation Details

### CLSCTX_LOCAL_SERVER

The shell uses `CLSCTX_LOCAL_SERVER` (0x4) when calling `CoCreateInstance()`.
This ensures providers run **out-of-process** in `dllhost.exe`.

**Why out-of-process?**
- **Isolation**: Provider crashes don't crash the Shell
- **Security**: Providers run as Interactive User and can access Credential Manager
- **No DLL loading**: Shell never loads provider DLLs, avoiding assembly conflicts

**P/Invoke Declaration** (ProviderFactory.cs):
```csharp
[DllImport("ole32.dll")]
private static extern int CoCreateInstance(
    [In] ref Guid rclsid,           // Provider CLSID
    object pUnkOuter,                // null (no aggregation)
    uint dwClsContext,               // CLSCTX_LOCAL_SERVER = 0x4
    [In] ref Guid riid,             // IID_IUnknown
    [MarshalAs(UnmanagedType.IUnknown)] out object ppv  // Returned interface
);
```

**Call site**:
```csharp
Guid clsid = new Guid("C7A1B2D3-E4F5-6789-AB01-CD23EF456789");
Guid iidUnknown = new Guid("00000000-0000-0000-C000-000000000046");
object provider;

int hr = CoCreateInstance(
    ref clsid, 
    null, 
    CLSCTX_LOCAL_SERVER,  // 0x4 = out-of-process
    ref iidUnknown, 
    out provider
);

IBigDriveDriveInfo driveInfo = provider as IBigDriveDriveInfo;
```

---

## Complete Example: Mounting Zip Provider

### Console Interaction

```
BD> mount

Available providers:

  [1] BigDrive.Provider.Sample
      CLSID: f8fe2e5a-e8b8-4207-bc04-ea4bcd4c4361

  [2] Zip
      CLSID: c7a1b2d3-e4f5-6789-ab01-cd23ef456789

Select provider number: 2
Enter drive name: MyPhotos

This provider requires the following parameters:

  Full path to the ZIP file (existing or new).
  ZipFilePath: C:\Photos\vacation2024.zip

Drive mounted successfully!

  Name:     MyPhotos
  GUID:     74bc469c-34d2-43b2-b263-232e07273782
  Provider: Zip

Use 'cd Y:' to access the new drive.
```

---

### Data Exchanged

**Step 1**: Shell reads registry:
```
HKLM\SOFTWARE\BigDrive\Providers\{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}\
  ├── id   = "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}"
  └── name = "Zip"
```

**Step 2**: Shell creates new drive ID and initial config:
```csharp
driveId = {74BC469C-34D2-43B2-B263-232E07273782}
providerClsid = {C7A1B2D3-E4F5-6789-AB01-CD23EF456789}
driveName = "MyPhotos"
```

**Step 3**: Shell → Provider COM call:
```
CoCreateInstance(CLSID={C7A1B2D3-...}, CLSCTX_LOCAL_SERVER)
  ↓ Returns provider instance
QueryInterface(IID_IBigDriveDriveInfo)
  ↓ Returns IBigDriveDriveInfo interface
GetDriveParameters()
  ↓ Returns JSON
```

**Step 4**: Provider → Shell response:
```json
[
  {
    "name": "ZipFilePath",
    "description": "Full path to the ZIP file.",
    "type": "file"
  }
]
```

**Step 5**: User enters value:
```
ZipFilePath = "C:\Photos\vacation2024.zip"
```

**Step 6**: Shell writes to registry:
```
HKLM\SOFTWARE\BigDrive\Drives\{74BC469C-34D2-43B2-B263-232E07273782}\
  ├── id          = "{74BC469C-34D2-43B2-B263-232E07273782}"
  ├── name        = "MyPhotos"
  ├── clsid       = "{C7A1B2D3-E4F5-6789-AB01-CD23EF456789}"
  └── ZipFilePath = "C:\Photos\vacation2024.zip"
```

**Step 7**: Drive letter assignment:
```
DriveLetterManager assigns: Y: → {74BC469C-...}
```

---

## Code Walkthrough

### Shell Side: MountCommand.cs

**Entry point**:
```csharp
public void Execute(ShellContext context, string[] args)
{
    // 1. Read available providers from registry
    List<ProviderConfiguration> providers = 
        ProviderManager.ReadProviders(CancellationToken.None).ToList();
    
    // 2. Interactive or direct mode
    if (args.Length == 0)
        InteractiveMount(context, providers);
    else
        DirectMount(context, providers, args);
}
```

**CreateDrive method**:
```csharp
private static void CreateDrive(
    ShellContext context, 
    ProviderConfiguration provider, 
    string driveName)
{
    // Generate new drive GUID
    Guid driveId = Guid.NewGuid();
    
    // Create configuration object
    DriveConfiguration driveConfig = new DriveConfiguration
    {
        Id = driveId,
        Name = driveName,
        CLSID = provider.Id
    };
    
    // Query provider for required parameters (COM call)
    Dictionary<string, string> properties = QueryDriveParameters(provider.Id);
    if (properties == null)
    {
        Console.WriteLine("Mount cancelled.");
        return;
    }
    
    driveConfig.Properties = properties;
    
    // Write to registry
    DriveManager.WriteConfiguration(driveConfig, CancellationToken.None);
    
    // Refresh and show drive letter
    context.RefreshDrives();
    char newLetter = context.DriveLetterManager.GetDriveLetter(driveId);
    Console.WriteLine("Use 'cd {0}:' to access the new drive.", newLetter);
}
```

**QueryDriveParameters method** (COM interaction):
```csharp
private static Dictionary<string, string> QueryDriveParameters(Guid providerClsid)
{
    Dictionary<string, string> properties = new Dictionary<string, string>();

    // Try to get IBigDriveDriveInfo from provider
    IBigDriveDriveInfo driveInfo = ProviderFactory.GetDriveInfoProvider(providerClsid);
    if (driveInfo == null)
    {
        // Provider doesn't need custom parameters
        return properties;
    }

    // Get parameter definitions from provider
    string json = driveInfo.GetDriveParameters();
    if (string.IsNullOrWhiteSpace(json))
        return properties;

    // Parse JSON
    JsonElement[] parameters = JsonSerializer.Deserialize<JsonElement[]>(json);

    // Prompt user for each parameter
    foreach (JsonElement param in parameters)
    {
        string name = param.GetProperty("name").GetString();
        string description = param.GetProperty("description").GetString();
        string type = param.GetProperty("type").GetString() ?? "string";

        Console.WriteLine("  {0}", description);
        Console.Write("  {0}: ", name);

        string value;
        if (string.Equals(type, "existing-file", StringComparison.OrdinalIgnoreCase))
        {
            value = ReadLineWithFileCompletion();
            if (!string.IsNullOrWhiteSpace(value) && !File.Exists(value))
            {
                Console.WriteLine("  Error: File does not exist: {0}", value);
                return null;  // Cancel mount
            }
        }
        else if (string.Equals(type, "filepath", StringComparison.OrdinalIgnoreCase) ||
                 string.Equals(type, "file", StringComparison.OrdinalIgnoreCase))
        {
            value = ReadLineWithFileCompletion();  // Tab completion, no validation
        }
        else
        {
            value = Console.ReadLine();  // Standard string input
        }

        properties[name] = value.Trim();
    }

    return properties;
}
```

---

### Provider Side: Provider.IBigDriveDriveInfo.cs

**Complete implementation for Zip Provider**:
```csharp
namespace BigDrive.Provider.Zip
{
    using System.Text.Json;
    using BigDrive.Interfaces;
    using BigDrive.Interfaces.Model;

    public partial class Provider
    {
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "ZipFilePath",
                    Description = "Full path to the ZIP file.",
                    Type = "file"
                }
            };

            return JsonSerializer.Serialize(parameters);
        }
    }
}
```

---

### Provider Side: Reading Configuration at Runtime

**When provider needs its config** (any interface method):
```csharp
public string[] EnumerateFiles(Guid driveGuid, string path)
{
    // 1. Read the ZipFilePath that was stored during mount
    string zipFilePath = DriveManager.ReadDriveProperty(
        driveGuid, 
        "ZipFilePath", 
        CancellationToken.None
    );
    
    // 2. Validate
    if (string.IsNullOrEmpty(zipFilePath) || !File.Exists(zipFilePath))
    {
        return Array.Empty<string>();
    }
    
    // 3. Use configuration to access storage
    using (ZipArchive archive = ZipFile.OpenRead(zipFilePath))
    {
        // ... enumerate files in ZIP
    }
}
```

**Under the hood**:
```
DriveManager.ReadDriveProperty(driveGuid, "ZipFilePath", ...)
  ↓
Registry.LocalMachine.OpenSubKey(
    $@"SOFTWARE\BigDrive\Drives\{driveGuid:B}"
)
  ↓
GetValue("ZipFilePath")
  ↓
Returns: "C:\MyArchive.zip"
```

---

## Registry Configuration Cache

Providers should **cache** registry reads if they call the same configuration multiple times
in rapid succession. The `ZipClientWrapper` pattern demonstrates this:

```csharp
// ZipClientWrapper.cs
private static readonly ConcurrentDictionary<Guid, ZipClientWrapper> DriveClients =
    new ConcurrentDictionary<Guid, ZipClientWrapper>();

private readonly Guid _driveGuid;
private readonly string _zipFilePath;  // Cached from registry

private ZipClientWrapper(Guid driveGuid)
{
    _driveGuid = driveGuid;
    // Read once during construction
    _zipFilePath = DriveManager.ReadDriveProperty(
        driveGuid, 
        "ZipFilePath", 
        CancellationToken.None
    );
}

public static ZipClientWrapper GetForDrive(Guid driveGuid)
{
    // Cache wrapper instances per drive
    return DriveClients.GetOrAdd(driveGuid, guid => new ZipClientWrapper(guid));
}
```

**Provider uses cached wrapper**:
```csharp
public string[] EnumerateFiles(Guid driveGuid, string path)
{
    ZipClientWrapper zipClient = GetZipClient(driveGuid);
    return zipClient.GetFiles(NormalizePath(path));  // Uses cached _zipFilePath
}
```

---

## Security Considerations

### Sensitive Data: Use Windows Credential Manager

For **sensitive** configuration (API keys, OAuth tokens, passwords), use
`DriveManager.WriteSecretProperty()` instead of storing in registry:

```csharp
// Shell side (e.g., after OAuth flow)
DriveManager.WriteSecretProperty(
    driveGuid, 
    "AccessToken",      // Secret name
    "oauth-token-xyz",  // Secret value
    CancellationToken.None
);
```

**Storage location**: Windows Credential Manager (per-user, encrypted)  
**Registry location**: None (secrets are **not** stored in registry)

**Provider reads secret**:
```csharp
string accessToken = DriveManager.ReadSecretProperty(
    driveGuid, 
    "AccessToken", 
    CancellationToken.None
);
```

**Why Credential Manager?**
- Encrypted by Windows
- Per-user (not system-wide)
- Not exposed in plain text in registry exports
- Requires Interactive User identity (providers run as logged-in user)

---

## Permission Requirements

### During Mount

| Operation | Permission Needed | Who |
|-----------|-------------------|-----|
| Read `HKLM\SOFTWARE\BigDrive\Providers` | Read | BigDrive.Shell |
| Write `HKLM\SOFTWARE\BigDrive\Drives\{GUID}` | Write | BigDrive.Shell |
| CoCreateInstance(Provider) | Execute | Shell + COM+ |

**Common issue**: `Access denied` when writing to `HKLM\SOFTWARE\BigDrive\Drives`

**Solutions**:
1. Run BigDrive.Shell as Administrator, or
2. During setup, grant "Users" group Full Control to `HKLM\SOFTWARE\BigDrive\Drives`

---

### During Runtime Operations

| Operation | Permission Needed | Who |
|-----------|-------------------|-----|
| Read `HKLM\SOFTWARE\BigDrive\Drives\{GUID}` | Read | Provider |
| Read Windows Credential Manager | Interactive User | Provider |
| Access local files (for Zip provider) | File system | Provider |

---

## Comparison: Mount vs Unmount

### Mount

1. Shell reads available providers from registry
2. Shell calls `IBigDriveDriveInfo.GetDriveParameters()` via COM
3. Shell collects user input for each parameter
4. Shell writes configuration to `HKLM\SOFTWARE\BigDrive\Drives\{GUID}`
5. Shell assigns drive letter locally

**No service involvement** — mount is entirely Shell ↔ Provider ↔ Registry

---

### Unmount

1. Shell reads drive configuration from registry
2. Shell calls `DriveManager.DeleteConfiguration(driveGuid)` to remove registry entry
3. Shell calls `DriveManager.DeleteAllSecrets(driveGuid)` to remove credentials
4. Shell refreshes drive letter assignments

**No provider involvement** — unmount is entirely Shell ↔ Registry

---

## Extending the Protocol

### Adding New Parameter Types

To support new parameter types (e.g., `"password"` for masked input, `"url"` for URL validation),
modify:

1. **Interface**: Update `DriveParameterDefinition.cs` documentation
2. **Shell**: `MountCommand.QueryDriveParameters()` — add handling for new type
3. **Providers**: Use new type in `GetDriveParameters()` return value

**Example**: Adding password type:
```csharp
// In DriveParameterDefinition.cs documentation
// "password" — Masked input (*** characters), not echoed to console

// In MountCommand.cs
if (string.Equals(type, "password", StringComparison.OrdinalIgnoreCase))
{
    value = ReadPasswordFromConsole();  // Masked input
}
else if (string.Equals(type, "existing-file", StringComparison.OrdinalIgnoreCase))
{
    value = ReadLineWithFileCompletion();
    if (!File.Exists(value))
    {
        Console.WriteLine("  Error: File does not exist.");
        return null;
    }
}
else if (string.Equals(type, "filepath", StringComparison.OrdinalIgnoreCase) ||
         string.Equals(type, "file", StringComparison.OrdinalIgnoreCase))
{
    value = ReadLineWithFileCompletion();
}
else
{
    value = Console.ReadLine();
}
```

---

### Provider Handling of Non-Existent File Paths

When using `"filepath"` type (allows new files), the provider must handle cases where
the file doesn't exist:

**Example: Zip Provider**
```csharp
// ZipClientWrapper.cs constructor
private ZipClientWrapper(Guid driveGuid)
{
    _driveGuid = driveGuid;
    _zipFilePath = DriveManager.ReadDriveProperty(driveGuid, "ZipFilePath", CancellationToken.None);

    // Create empty ZIP if it doesn't exist
    EnsureZipFileExists();
}

private void EnsureZipFileExists()
{
    if (string.IsNullOrEmpty(_zipFilePath) || File.Exists(_zipFilePath))
        return;

    // Create parent directories if needed
    string directory = Path.GetDirectoryName(_zipFilePath);
    if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
    {
        Directory.CreateDirectory(directory);
    }

    // Create empty ZIP archive
    using (ZipArchive archive = ZipFile.Open(_zipFilePath, ZipArchiveMode.Create))
    {
        // Empty archive
    }
}
```

---

## See Also

- [Architecture Overview](overview.md) — System architecture and components
- [Installation Architecture](installation.md) — COM+ registration and setup
- [Provider Development Guide](../ProviderDevelopmentGuide.md) — Creating providers
- [BigDrive.Shell User Guide](../BigDrive.Shell.UserGuide.md) — mount/unmount commands
- **Interface Definitions**:
  - `src/Interfaces/IBigDriveDriveInfo.cs` — Parameter discovery interface
  - `src/Interfaces/IBigDriveRegistration.cs` — Provider registration callbacks
  - `src/Interfaces/Model/DriveParameterDefinition.cs` — Parameter definition model
- **Implementation Examples**:
  - `src/BigDrive.Provider.Zip/Provider.IBigDriveDriveInfo.cs` — Zip provider parameters
  - `src/BigDrive.Shell/Commands/MountCommand.cs` — Shell-side mount logic
