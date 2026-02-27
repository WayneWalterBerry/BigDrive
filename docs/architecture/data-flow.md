# BigDrive Data Flow

This document describes the request/response flows for common BigDrive operations.

---

## Enumeration Flow: `dir` Command

### User Action

```
Z:\> dir
```

### Flow Diagram

```
┌─────────────────┐
│ User types:     │
│ Z:\> dir        │
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. DriveLetterManager: Z: → DriveGuid {6369DDE1-...}  │
│  2. DriveManager.ReadConfiguration(DriveGuid)           │
│     → Returns CLSID {B3D8F2A1-...}                     │
│  3. ProviderFactory.GetEnumerateProvider(DriveGuid)     │
│     → CoCreateInstance(CLSID)                           │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
              (out-of-process)
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (Provider Process)                          │
│                                                         │
│  Provider.Flickr                                        │
│  4. IBigDriveEnumerate.EnumerateFolders(DriveGuid, "\")│
│     → Reads drive config (API keys, tokens)             │
│     → Calls Flickr API: GET /photosets                  │
│     → Returns ["Vacation 2024", "Family Photos"]        │
│                                                         │
│  5. IBigDriveEnumerate.EnumerateFiles(DriveGuid, "\")  │
│     → Calls Flickr API: GET /photos (root level)        │
│     → Returns []                                        │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  6. Format output:                                      │
│     - Display folder count                              │
│     - Display each folder name with <DIR> prefix        │
│     - Display file count                                │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Output:         │
│                 │
│ Directory of Z:\│
│                 │
│ <DIR> Vacation  │
│ <DIR> Family    │
│                 │
│ 2 folders       │
│ 0 files         │
└─────────────────┘
```

### Step-by-Step Details

**Step 1: Drive Letter Resolution**
```csharp
// Shell/DriveLetterManager.cs
Guid driveGuid = DriveLetterManager.GetDriveGuidFromLetter("Z");
// Returns: {6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}
```

**Step 2: Read Drive Configuration**
```csharp
// ConfigProvider/DriveManager.cs
Guid clsid = DriveManager.ReadProviderClsid(driveGuid);
// Reads from: HKLM\SOFTWARE\BigDrive\Drives\{6369DDE1-...}\clsid
// Returns: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}
```

**Step 3: Activate Provider**
```csharp
// Shell/ProviderFactory.cs
Type providerType = Type.GetTypeFromCLSID(clsid);
IBigDriveEnumerate enumerator = (IBigDriveEnumerate)Activator.CreateInstance(providerType);
// COM+ creates dllhost.exe process and loads Provider.Flickr.dll
```

**Step 4-5: Enumerate Contents**
```csharp
// Shell/Commands/DirCommand.cs
string[] folders = enumerator.EnumerateFolders(driveGuid, @"\");
string[] files = enumerator.EnumerateFiles(driveGuid, @"\");
```

**Step 6: Display Results**
```csharp
Console.WriteLine($"Directory of {driveLetter}:\\{path}");
Console.WriteLine();
foreach (string folder in folders)
{
    Console.WriteLine($"<DIR> {folder}");
}
Console.WriteLine();
Console.WriteLine($"{folders.Length} folders, {files.Length} files");
```

---

## File Copy Flow: Local → BigDrive

### User Action

```
C:\> bigdrive copy photo.jpg Z:\
```

### Flow Diagram

```
┌──────────────────────────┐
│ User types:              │
│ copy photo.jpg Z:\       │
└────────┬─────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. Parse source: C:\photo.jpg → Local file             │
│  2. Parse dest: Z:\ → BigDrive path                     │
│  3. Resolve Z: → DriveGuid {6369DDE1-...}               │
│  4. Get provider CLSID from drive config                │
│  5. Activate provider via COM+                          │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (Provider Process)                          │
│                                                         │
│  Provider.Flickr                                        │
│  6. Shell calls IBigDriveFileOperations                 │
│     .CopyFileToBigDrive(driveGuid,                      │
│                         "C:\\photo.jpg",                │
│                         "\\photo.jpg")                  │
│                                                         │
│  7. Provider reads C:\photo.jpg (local filesystem)      │
│  8. Provider uploads to Flickr API: POST /photos/upload│
│  9. Provider returns success                            │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Shell Output:   │
│ 1 file copied   │
└─────────────────┘
```

### Alternative: Using IStream

If provider implements `IBigDriveFileData` but not `IBigDriveFileOperations`, Shell can use streaming:

```
┌──────────────────────────┐
│ Shell                    │
│                          │
│  1. Open local file for  │
│     reading              │
│  2. Create temp file on  │
│     BigDrive via HTTP    │
│  3. Stream chunks:       │
│     Local → Provider     │
│  4. Finalize upload      │
└──────────────────────────┘
```

---

## File Copy Flow: BigDrive → Local

### User Action

```
Z:\> copy photo.jpg C:\
```

### Flow Diagram

```
┌──────────────────────────┐
│ User types:              │
│ copy photo.jpg C:\       │
└────────┬─────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. Parse source: Z:\photo.jpg → BigDrive file          │
│  2. Parse dest: C:\ → Local path                        │
│  3. Resolve Z: → DriveGuid                              │
│  4. Activate provider                                   │
│  5. Call IBigDriveFileData.GetFileData(driveGuid,       │
│                                        "\photo.jpg")    │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (Provider Process)                          │
│                                                         │
│  Provider.Flickr                                        │
│  6. Provider calls Flickr API: GET /photos/{id}/sizes   │
│  7. Provider gets download URL for original size        │
│  8. Provider creates IStream wrapper around HTTP stream │
│  9. Provider returns IStream                            │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  10. Shell opens C:\photo.jpg for writing               │
│  11. Shell reads from IStream in 64KB chunks            │
│  12. Shell writes chunks to local file                  │
│  13. Shell closes IStream and local file                │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Shell Output:   │
│ 1 file copied   │
└─────────────────┘
```

### Step-by-Step Details

**Step 5: Get File Stream**
```csharp
// Shell/Commands/CopyCommand.cs
IBigDriveFileData fileData = provider as IBigDriveFileData;
IStream stream = fileData.GetFileData(driveGuid, @"\photo.jpg");
```

**Step 8: Create IStream Wrapper**
```csharp
// Provider.Flickr/Provider.cs
public IStream GetFileData(Guid driveGuid, string path)
{
    // Get download URL from Flickr API
    string downloadUrl = GetDownloadUrl(driveGuid, path);
    
    // Create HTTP stream
    HttpWebRequest request = (HttpWebRequest)WebRequest.Create(downloadUrl);
    HttpWebResponse response = (HttpWebResponse)request.GetResponse();
    Stream httpStream = response.GetResponseStream();
    
    // Wrap in COM IStream
    return new IStreamWrapper(httpStream);
}
```

**Step 11-12: Stream Copy**
```csharp
// Shell/FileTransferService.cs
byte[] buffer = new byte[65536]; // 64KB
IntPtr bytesRead = Marshal.AllocHGlobal(sizeof(int));

while (true)
{
    stream.Read(buffer, buffer.Length, bytesRead);
    int read = Marshal.ReadInt32(bytesRead);
    if (read == 0) break;
    
    localFile.Write(buffer, 0, read);
}

Marshal.FreeHGlobal(bytesRead);
```

---

## Drive Mount Flow

### User Action

```
> bigdrive mount Z: flickr "My Flickr"
```

### Flow Diagram

```
┌──────────────────────────┐
│ User types:              │
│ mount Z: flickr "My..."  │
└────────┬─────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. Parse command: letter=Z, provider=flickr, name=...  │
│  2. Lookup provider CLSID from provider name:           │
│     ProviderManager.GetProviderByName("flickr")         │
│     → Returns CLSID {B3D8F2A1-...}                     │
│  3. Build JSON configuration:                           │
│     {                                                   │
│       "name": "My Flickr",                              │
│       "letter": "Z",                                    │
│       "clsid": "{B3D8F2A1-...}",                        │
│       "properties": { ... }                             │
│     }                                                   │
│  4. Activate BigDrive.Service via COM+                  │
│  5. Call IBigDriveProvision.CreateFromConfiguration()   │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
              (elevated process)
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (BigDrive.Service)                          │
│ Identity: BigDriveInstaller (elevated)                  │
│                                                         │
│  6. Service generates new DriveGuid                     │
│  7. Service writes to registry:                         │
│     HKLM\SOFTWARE\BigDrive\Drives\{NewGuid}\            │
│       ├── id    = "{NewGuid}"                           │
│       ├── name  = "My Flickr"                           │
│       ├── letter = "Z"                                  │
│       └── clsid = "{B3D8F2A1-...}"                      │
│                                                         │
│  8. Service writes shell namespace:                     │
│     HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\    │
│       Explorer\MyComputer\NameSpace\{NewGuid}\          │
│         └── (Default) = "My Flickr"                     │
│                                                         │
│  9. Service refreshes Explorer: SHChangeNotify()        │
│ 10. Service returns new DriveGuid                       │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│ 11. Shell displays success message                      │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Shell Output:   │
│ Drive Z: mounted│
│ successfully    │
└─────────────────┘
```

### Registry Changes

**Before mount:**
```
HKLM\SOFTWARE\BigDrive\Drives\
  (empty or other drives)
```

**After mount:**
```
HKLM\SOFTWARE\BigDrive\Drives\
└── {6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}\
    ├── id    = "{6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}"
    ├── name  = "My Flickr"
    ├── letter = "Z"
    └── clsid = "{B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}"

HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\
└── {6369DDE1-C5B8-4A7F-9E2D-3C1B4A5E6F7A}\
    └── (Default) = "My Flickr"
```

**Explorer refresh:** Explorer automatically picks up the new namespace entry and displays "My Flickr" under "This PC".

---

## Drive Unmount Flow

### User Action

```
> bigdrive unmount Z:
```

### Flow Diagram

```
┌──────────────────────────┐
│ User types:              │
│ unmount Z:               │
└────────┬─────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. Resolve Z: → DriveGuid {6369DDE1-...}               │
│  2. Activate BigDrive.Service via COM+                  │
│  3. Call IBigDriveProvision.UnmountDrive(driveGuid)     │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (BigDrive.Service)                          │
│                                                         │
│  4. Service deletes registry keys:                      │
│     - HKLM\SOFTWARE\BigDrive\Drives\{DriveGuid}\        │
│     - HKLM\...\MyComputer\NameSpace\{DriveGuid}\        │
│                                                         │
│  5. Service refreshes Explorer: SHChangeNotify()        │
│  6. Service returns success                             │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Shell Output:   │
│ Drive Z:        │
│ unmounted       │
└─────────────────┘
```

**Explorer refresh:** Explorer automatically removes "My Flickr" from "This PC".

---

## Authentication Flow (OAuth)

### User Action

```
Z:\> auth
```

### Flow Diagram

```
┌──────────────────────────┐
│ User types: auth         │
└────────┬─────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  1. Resolve current drive → DriveGuid                   │
│  2. Activate provider                                   │
│  3. Check if implements IBigDriveAuthentication         │
│  4. Call GetAuthenticationInfo(driveGuid)               │
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (Provider)                                  │
│                                                         │
│  5. Provider returns JSON:                              │
│     {                                                   │
│       "flowType": "OAuth1",                             │
│       "requestTokenUrl": "...",                         │
│       "authorizeUrl": "...",                            │
│       "accessTokenUrl": "..."                           │
│     }                                                   │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ BigDrive.Shell                                          │
│                                                         │
│  6. Shell starts local HTTP server for callback         │
│  7. Shell performs OAuth1 three-legged flow:            │
│     a) Request token from provider                      │
│     b) Open browser to authorization URL                │
│     c) Wait for callback with verifier                  │
│     d) Exchange verifier for access token               │
│  8. Shell builds auth data JSON:                        │
│     {                                                   │
│       "oauthToken": "...",                              │
│       "oauthSecret": "..."                              │
│     }                                                   │
│  9. Shell calls OnAuthenticationComplete(driveGuid, json)│
└─────────────────────────┬───────────────────────────────┘
                          │
              COM+ Activation
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ dllhost.exe (Provider)                                  │
│                                                         │
│ 10. Provider saves tokens to drive config:              │
│     DriveManager.WriteDriveProperty(driveGuid,          │
│                                     "FlickrOAuthToken", │
│                                     token)              │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────┐
│ Shell Output:   │
│ Authentication  │
│ successful      │
└─────────────────┘
```

**Registry Changes:**
```
HKLM\SOFTWARE\BigDrive\Drives\{6369DDE1-...}\
├── ... (existing keys)
├── FlickrOAuthToken  = "72157..."
└── FlickrOAuthSecret = "abc123..."
```

---

## Configuration Lookup Flow

### Provider reads configuration value

```
Provider needs FlickrApiKey for drive {6369DDE1-...}
```

### Lookup Sequence

```
┌─────────────────────────────────────────────────────────┐
│ Provider.Flickr                                         │
│                                                         │
│  1. Read drive-specific value:                          │
│     DriveManager.ReadDriveProperty(driveGuid,           │
│                                    "FlickrApiKey")      │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ ConfigProvider.DriveManager                             │
│                                                         │
│  2. Query registry:                                     │
│     HKLM\SOFTWARE\BigDrive\Drives\{6369DDE1-...}\       │
│       FlickrApiKey                                      │
│                                                         │
│  3. If found → return value                             │
│  4. If not found → return null                          │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼ (if null)
┌─────────────────────────────────────────────────────────┐
│ Provider.Flickr                                         │
│                                                         │
│  5. Fallback to provider default:                       │
│     ProviderManager.ReadProviderProperty(               │
│         providerClsid,                                  │
│         "FlickrApiKey")                                 │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│ ConfigProvider.ProviderManager                          │
│                                                         │
│  6. Query registry:                                     │
│     HKLM\SOFTWARE\BigDrive\Providers\{B3D8F2A1-...}\    │
│       FlickrApiKey                                      │
│                                                         │
│  7. If found → return value                             │
│  8. If not found → return null                          │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼ (if null)
┌─────────────────────────────────────────────────────────┐
│ Provider.Flickr                                         │
│                                                         │
│  9. Use hard-coded default:                             │
│     apiKey = "default-hardcoded-key";                   │
└─────────────────────────────────────────────────────────┘
```

**Priority:** Drive-specific → Provider default → Hard-coded

---

## See Also

- [Overview](overview.md) — High-level architecture
- [Interfaces](interfaces.md) — Interface definitions
- [Registry Structure](registry.md) — Configuration storage
- [Process Isolation](process-isolation.md) — COM+ activation details
