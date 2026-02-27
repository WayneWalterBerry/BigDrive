# Troubleshooting BigDrive Provider Development

This guide covers common errors encountered when developing BigDrive providers and their solutions.

---

## Registration Errors (regsvcs.exe)

### Error: "Access to the registry key 'HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\...' is denied"

**Full error message:**
```
Failed to register assembly 'YourProvider, Version=1.0.0.0'
Exception has been thrown by the target of an invocation.
Access to the registry key 'HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive\Providers\{GUID}' is denied.
```

**Cause:** The `regsvcs.exe` PostBuild event needs **Administrator privileges** to write to
HKEY_LOCAL_MACHINE registry keys.

**Solution:**

**Option 1: Run Visual Studio as Administrator** (Recommended for development)
1. Close Visual Studio
2. Right-click Visual Studio icon → Run as Administrator
3. Rebuild your solution

**Option 2: Manually register the provider**
```powershell
# Open PowerShell or CMD as Administrator
cd "path\to\your\provider\bin\Debug\net472\"
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe YourProvider.dll
```

**Option 3: Disable PostBuild registration temporarily**

In your `.csproj`, comment out the PostBuild event:
```xml
<!--
<Target Name="PostBuild" AfterTargets="PostBuildEvent">
  <Exec Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe &quot;$(TargetPath)&quot;" />
</Target>
-->
```

Then register manually when needed.

---

### Error: "Could not load file or assembly 'System.Text.Json, Version=...'"

**Full error message:**
```
Failed to register assembly 'YourProvider'
Exception has been thrown by the target of an invocation.
Could not load file or assembly 'System.Text.Json, Version=4.0.1.2, Culture=neutral, PublicKeyToken=cc7b13ffcd2ddd51'
or one of its dependencies. The located assembly's manifest definition does not match the assembly reference. (HRESULT: 0x80131040)
```

**Cause:** Version mismatch between requested assembly version and actual version in bin folder.

**Solution:**

**1. Check actual DLL version in bin folder:**
```powershell
dir "bin\Debug\net472\System.Text.Json.dll"
# Check Properties → Details → File version
```

**2. Update app.config binding redirect:**
```xml
<dependentAssembly>
    <assemblyIdentity name="System.Text.Json" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
    <!-- Update newVersion to match actual DLL version -->
    <bindingRedirect oldVersion="0.0.0.0-9.0.0.11" newVersion="9.0.0.11" />
</dependentAssembly>
```

**3. Ensure AssemblyResolver includes the assembly:**
```csharp
string[] managedAssemblies = new string[]
{
    "System.Text.Json", // Add if missing
    // ...
};
```

**4. Rebuild:**
```powershell
msbuild YourProvider.csproj /t:Rebuild
```

**See:** [nuget-dependencies.md](nuget-dependencies.md) for complete setup guide.

---

### Error: "The type initializer for 'Provider' threw an exception"

**Full error message:**
```
Failed to register assembly 'YourProvider'
System.TypeInitializationException: The type initializer for 'Provider' threw an exception.
```

**Cause:** Exception in the Provider static constructor (usually AssemblyResolver initialization).

**Solution:**

**1. Check Windows Event Log for inner exception:**
```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 10
```

**2. Verify AssemblyResolver.cs exists and compiles:**
- Check that `AssemblyResolver.cs` is in your project
- Ensure it's included in the .csproj (not excluded)
- Look for syntax errors

**3. Verify static constructor in Provider.cs:**
```csharp
static Provider()
{
    AssemblyResolver.Initialize(); // Must be present
}
```

**4. Check for circular dependencies:**
- Don't reference Provider members from static constructor
- Don't initialize complex objects in static constructor

---

### Error: "Failed to load the type 'YourProvider' from assembly"

**Full error message:**
```
Failed to load the type 'YourService.Provider' from assembly 'YourProvider'
Could not load file or assembly 'BigDrive.Interfaces' or one of its dependencies.
```

**Cause:** Missing project or assembly references.

**Solution:**

**1. Verify project references in .csproj:**
```xml
<ItemGroup>
  <ProjectReference Include="..\Interfaces\BigDrive.Interfaces.csproj" />
  <ProjectReference Include="..\ConfigProvider\BigDrive.ConfigProvider.csproj" />
  <ProjectReference Include="..\BigDrive.Service\BigDrive.Service.csproj" />
</ItemGroup>
```

**2. Verify framework references:**
```xml
<ItemGroup>
  <Reference Include="System.EnterpriseServices" />
</ItemGroup>
```

**3. Clean and rebuild dependencies first:**
```powershell
msbuild Interfaces\BigDrive.Interfaces.csproj /t:Rebuild
msbuild ConfigProvider\BigDrive.ConfigProvider.csproj /t:Rebuild
msbuild YourProvider\YourProvider.csproj /t:Rebuild
```

---

## Runtime Errors (dllhost.exe)

### Error: Methods fail with "Could not load file or assembly"

**Symptom:** Provider registers successfully, but when called from BigDrive Shell:
```
Error: Could not load file or assembly 'YourPackage, Version=1.0.0.0' or one of its dependencies.
```

**Cause:** Missing assembly in the `AssemblyResolver.managedAssemblies` array.

**Solution:**

**1. Check Windows Event Log:**
```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 10 | Format-List
```

**2. Find the specific assembly name that failed**

**3. Add to managedAssemblies in AssemblyResolver.cs:**
```csharp
string[] managedAssemblies = new string[]
{
    "YourPackage", // Add the missing assembly name
    // ... existing assemblies
};
```

**4. Verify DLL is in bin folder:**
```powershell
dir "bin\Debug\net472\YourPackage.dll"
```

If missing, add to .csproj:
```xml
<ItemGroup>
  <PackageReference Include="YourPackage" Version="1.0.0" />
</ItemGroup>
```

**5. Ensure CopyLocalLockFileAssemblies is enabled:**
```xml
<PropertyGroup>
  <CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
</PropertyGroup>
```

---

### Error: "Exception has been thrown by the target of an invocation"

**Symptom:** Generic wrapper exception with inner exception details.

**Cause:** Various - check inner exception in Event Log.

**Solution:**

**1. Check Windows Event Log for inner exception:**
```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 10 |
  Where-Object {$_.EntryType -eq "Error"} | Format-List
```

**2. Common inner exceptions:**
- "Could not load file or assembly" → Add to AssemblyResolver
- "Access to registry key denied" → Run as Administrator
- "Object reference not set" → Check null handling in code
- "Invalid cast" → Check COM interface signatures match

---

## Build Errors

### Error: "CS0535: 'Provider' does not implement interface member"

**Symptom:**
```
CS0535: 'Provider' does not implement interface member 'IBigDriveEnumerate.EnumerateFolders(Guid, string)'
```

**Cause:** Interface signature mismatch - your implementation doesn't match the interface definition.

**Solution:**

**1. Check the interface definition:**
```csharp
// In BigDrive.Interfaces project
public interface IBigDriveEnumerate
{
    string[] EnumerateFolders(Guid driveGuid, string path); // Correct signature
}
```

**2. Fix your implementation:**
```csharp
// WRONG - doesn't match interface
public int EnumerateFolders(Guid driveGuid, string path, out string[] folders)

// CORRECT - matches interface
public string[] EnumerateFolders(Guid driveGuid, string path)
```

**See:** [interfaces.md](interfaces.md) for correct signatures.

---

### Error: "CS0738: 'Provider' does not implement interface member ... does not have the matching return type"

**Symptom:**
```
CS0738: 'Provider' does not implement interface member 'IBigDriveRegistration.Register()'. 
'Provider.Register()' cannot implement 'IBigDriveRegistration.Register()' because it does not have the matching return type of 'void'.
```

**Cause:** Method returns `int` instead of `void` (or vice versa).

**Solution:**

Check interface definition and match return type exactly:

```csharp
// Interface definition
public interface IBigDriveRegistration
{
    void Register();    // Returns void
    void Unregister();  // Returns void
}

// Your implementation must match
public void Register()    // Not 'int'
{
    // ...
}
```

---

### Error: "CS0246: The type or namespace name 'DriveParameterDefinition' could not be found"

**Symptom:**
```
CS0246: The type or namespace name 'DriveParameterDefinition' could not be found
```

**Cause:** Missing `using` directive for `BigDrive.Interfaces.Model`.

**Solution:**

Add to the top of your file:
```csharp
using BigDrive.Interfaces.Model;
```

---

## OAuth Errors

### Error: "FlickrApiKey and FlickrApiSecret must be set"

**Symptom:** When calling `bigdrive login` for OAuth 1.0a provider:
```
Error: FlickrApiKey and FlickrApiSecret must be set. Use 'bigdrive secret set FlickrApiKey "..."'
```

**Cause:** OAuth 1.0a requires consumer key/secret before login flow.

**Solution:**

**1. Get API credentials from service's developer portal**
- Flickr: https://www.flickr.com/services/apps/create/
- Twitter: https://developer.twitter.com/

**2. Set credentials via BigDrive Shell:**
```sh
bigdrive secret set FlickrApiKey "your-api-key-here"
bigdrive secret set FlickrApiSecret "your-api-secret-here"
```

**3. Then run login:**
```sh
bigdrive login
```

---

### Error: "Authentication failed: Invalid redirect URI"

**Symptom:** Browser shows error after authorization:
```
Error: redirect_uri_mismatch
The redirect URI provided does not match a registered redirect URI.
```

**Cause:** OAuth application not configured for `http://localhost:8080/callback`.

**Solution:**

**1. Go to your service's developer portal** (Azure Portal, Google Console, etc.)

**2. Add redirect URI to your OAuth application:**
```
http://localhost:8080/callback
```

**3. For production, also add:**
```
http://127.0.0.1:8080/callback
```

---

## COM+ Errors

### Error: "COM object that has been separated from its underlying RCW"

**Symptom:**
```
System.InvalidComObjectException: COM object that has been separated from its underlying RCW cannot be used.
```

**Cause:** COM object used after being released or across apartment boundaries.

**Solution:**

**1. Don't store COM interface references across calls**

```csharp
// WRONG
private IStream _cachedStream; // Don't cache COM interfaces

// CORRECT
public int GetFileData(Guid driveGuid, string path, out IStream stream)
{
    stream = new ComStream(fileStream); // Create fresh each time
    return 0;
}
```

**2. Release COM objects properly:**
```csharp
if (comObject != null && Marshal.IsComObject(comObject))
{
    Marshal.ReleaseComObject(comObject);
}
```

---

### Error: "Retrieving the COM class factory for component with CLSID {GUID} failed"

**Symptom:**
```
Retrieving the COM class factory for component with CLSID {F8E7D6C5-...} failed due to the following error: 80040154
Class not registered (Exception from HRESULT: 0x80040154 (REGDB_E_CLASSNOTREG)).
```

**Cause:** Provider not registered or registration incomplete.

**Solution:**

**1. Register the provider as Administrator:**
```powershell
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "YourProvider.dll"
```

**2. Verify COM+ application exists:**
- Open Component Services (dcomcnfg.exe)
- Navigate to: Component Services → Computers → My Computer → COM+ Applications
- Look for "BigDrive.Provider.YourService"

**3. Check CLSID registration:**
```powershell
# Check registry
reg query "HKCR\CLSID\{YOUR-PROVIDER-CLSID}"
```

**4. If still failing, try unregister then re-register:**
```powershell
regsvcs.exe /u YourProvider.dll
regsvcs.exe YourProvider.dll
```

---

## NuGet / Assembly Loading Errors

### Error: "Method not found: 'System.Span`1<T> System.Memory`1<T>.get_Span()'"

**Symptom:**
```
System.MissingMethodException: Method not found: 'System.Span`1<!!0> System.Memory`1<!!0>.get_Span()'.
```

**Cause:** Version conflict between System.Memory versions.

**Solution:**

**1. Add binding redirect to app.config:**
```xml
<dependentAssembly>
    <assemblyIdentity name="System.Memory" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
    <bindingRedirect oldVersion="0.0.0.0-4.0.5.0" newVersion="4.0.1.2" />
</dependentAssembly>
```

**2. Ensure System.Memory is in AssemblyResolver:**
```csharp
string[] managedAssemblies = new string[]
{
    "System.Memory", // Add if missing
    // ...
};
```

---

### Error: "System.Text.Json.JsonException: The JSON value could not be converted"

**Symptom:**
```
System.Text.Json.JsonException: The JSON value could not be converted to System.String.
```

**Cause:** JSON deserialization failure in `GetDriveParameters()` or configuration parsing.

**Solution:**

**1. Verify JSON format in GetDriveParameters():**
```csharp
public string GetDriveParameters()
{
    DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
    {
        new DriveParameterDefinition
        {
            Name = "ParamName",        // Must be string
            Description = "...",        // Must be string
            Type = "string"            // Must be "string", "existing-file", or "filepath"
        }
    };

    return JsonSerializer.Serialize(parameters);
}
```

**2. Test JSON output manually:**
```csharp
string json = provider.GetDriveParameters();
Console.WriteLine(json); // Should be: [{"name":"...","description":"...","type":"..."}]
```

---

## Path Handling Errors

### Error: "Path format is invalid" or "Illegal characters in path"

**Symptom:**
```
System.ArgumentException: Illegal characters in path.
```

**Cause:** Path contains invalid Windows filename characters or incorrect separator.

**Solution:**

**1. Sanitize folder/file names returned by EnumerateFolders/EnumerateFiles:**
```csharp
private static string SanitizeName(string name)
{
    if (string.IsNullOrEmpty(name))
    {
        return "Untitled";
    }

    // Remove invalid characters
    char[] invalidChars = Path.GetInvalidFileNameChars();
    foreach (char c in invalidChars)
    {
        name = name.Replace(c, '_');
    }

    return name;
}
```

**2. Normalize paths in your methods:**
```csharp
private static string NormalizePath(string path)
{
    if (string.IsNullOrEmpty(path) || path == "\\" || path == "/" || path == "//")
    {
        return string.Empty; // Root
    }

    return path.Trim('\\', '/').Replace('\\', '/');
}
```

**3. Return names only, not paths:**
```csharp
// WRONG
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    return new[] { "\\folder\\subfolder" }; // Don't return paths!
}

// CORRECT
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    return new[] { "subfolder" }; // Return names only
}
```

---

## Shell Integration Errors

### Error: "Drive 'MyDrive' not found"

**Symptom:** After creating a drive:
```sh
bigdrive cd MyDrive
Error: Drive 'MyDrive' not found
```

**Cause:** Drive registration incomplete or registry key missing.

**Solution:**

**1. Check drive registry:**
```powershell
reg query "HKCU\SOFTWARE\BigDrive\Drives"
```

Should show subkey with drive GUID.

**2. Recreate the drive:**
```sh
bigdrive drive delete MyDrive
bigdrive drive create --provider YourService --name MyDrive
```

**3. Check provider registration:**
```powershell
reg query "HKLM\SOFTWARE\BigDrive\Providers"
```

Should show subkey with provider CLSID.

---

### Error: "Provider returned null or empty array"

**Symptom:**
```sh
bigdrive ls
# (no output, empty directory)
```

**Cause:** Your `EnumerateFolders` or `EnumerateFiles` returning null or failing silently.

**Solution:**

**1. Add logging to your enumerate methods:**
```csharp
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    try
    {
        DefaultTraceSource.TraceInformation($"EnumerateFolders: path='{path}'");

        var result = GetClient(driveGuid).GetFolders(NormalizePath(path));

        DefaultTraceSource.TraceInformation($"EnumerateFolders: returned {result.Length} folders");
        return result;
    }
    catch (Exception ex)
    {
        DefaultTraceSource.TraceError($"EnumerateFolders failed: {ex.Message}");
        return Array.Empty<string>(); // Never return null!
    }
}
```

**2. Check Event Log for errors:**
```powershell
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 20
```

**3. Test provider in isolation:**
```csharp
// Simple test harness
Provider p = new Provider();
Guid testGuid = Guid.Parse("{your-test-drive-guid}");
string[] folders = p.EnumerateFolders(testGuid, "\\");
Console.WriteLine($"Found {folders.Length} folders");
```

---

## Configuration Errors

### Error: "The drive parameter 'XxxFilePath' is required but was not provided"

**Symptom:**
```sh
bigdrive drive create --provider Iso --name "Test"
Error: The drive parameter 'IsoFilePath' is required but was not provided.
```

**Cause:** Drive parameter not set after creation.

**Solution:**

**1. Create drive first:**
```sh
bigdrive drive create --provider Iso --name "Test"
```

**2. Set required parameters:**
```sh
bigdrive set IsoFilePath "C:\path\to\file.iso"
```

**3. Verify parameter is set:**
```sh
bigdrive status
```

---

### Error: "ProviderConfigurationFactory.Create() returned null"

**Symptom:**
```
System.NullReferenceException: Object reference not set to an instance of an object.
```

**Cause:** `ProviderConfigurationFactory.Create()` method not implemented or returns null.

**Solution:**

**Create ProviderConfigurationFactory.cs:**
```csharp
using BigDrive.ConfigProvider.Model;

namespace BigDrive.Provider.YourService
{
    internal static class ProviderConfigurationFactory
    {
        public static ProviderConfiguration Create()
        {
            return new ProviderConfiguration
            {
                Id = Provider.CLSID,
                Name = "YourService"
            };
        }
    }
}
```

---

## Debugging Tips

### Enable Detailed Logging

**1. Check Event Log during operations:**
```powershell
# Real-time monitoring
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 1 -Wait
```

**2. Add verbose logging to your methods:**
```csharp
DefaultTraceSource.TraceInformation($"MethodName: param1={param1}, param2={param2}");

// ... your code

DefaultTraceSource.TraceInformation($"MethodName: returning {result.Length} items");
```

### Attach Debugger to dllhost.exe

**1. Build in Debug configuration**

**2. Start a BigDrive Shell operation that triggers your provider:**
```sh
bigdrive ls
```

**3. In Visual Studio:**
- Debug → Attach to Process
- Find `dllhost.exe` (may be multiple, look for command line with your provider DLL)
- Set breakpoints in your Provider methods

**4. Trigger operation again:**
```sh
bigdrive ls
```

Debugger should hit your breakpoints.

### Test Provider Directly (Without Shell)

Create a simple console app to test your provider in isolation:

```csharp
using System;
using BigDrive.Provider.YourService;

class Program
{
    static void Main()
    {
        Provider provider = new Provider();
        Guid testDriveGuid = Guid.Parse("{your-test-guid}");

        // Test enumeration
        string[] folders = provider.EnumerateFolders(testDriveGuid, "\\");
        Console.WriteLine($"Found {folders.Length} folders");

        // Test file info
        DateTime modTime = provider.LastModifiedTime(testDriveGuid, "\\test.txt");
        Console.WriteLine($"Last modified: {modTime}");
    }
}
```

This bypasses COM+ activation and lets you debug directly.

---

## Performance Issues

### Issue: Slow EnumerateFolders/EnumerateFiles

**Symptom:** Directory listings take 5+ seconds.

**Solution:**

**1. Implement caching in your client wrapper:**
```csharp
public class YourServiceClientWrapper
{
    private readonly Dictionary<string, CachedResult> _cache = new Dictionary<string, CachedResult>();

    public string[] GetFolders(string path)
    {
        if (_cache.TryGetValue(path, out CachedResult cached))
        {
            if (DateTime.UtcNow - cached.Timestamp < TimeSpan.FromMinutes(5))
            {
                return cached.Folders;
            }
        }

        // Cache miss - fetch from API
        string[] folders = FetchFoldersFromApi(path);
        _cache[path] = new CachedResult { Folders = folders, Timestamp = DateTime.UtcNow };
        return folders;
    }
}
```

**2. Use batch API calls if available:**
```csharp
// Fetch metadata for all files in one API call instead of per-file
public Dictionary<string, FileMetadata> GetAllFileMetadata(string path)
{
    return _apiClient.BatchGetMetadata(path);
}
```

---

### Issue: High memory usage in dllhost.exe

**Symptom:** dllhost.exe process grows to 500+ MB.

**Solution:**

**1. Dispose streams properly:**
```csharp
public int GetFileData(Guid driveGuid, string path, out IStream stream)
{
    Stream fileStream = null;
    try
    {
        fileStream = GetClient(driveGuid).OpenFile(path);
        stream = new ComStream(fileStream);
        fileStream = null; // ComStream now owns it
        return 0;
    }
    catch
    {
        fileStream?.Dispose(); // Clean up on error
        stream = null;
        return -1;
    }
}
```

**2. Implement IDisposable in your client wrapper:**
```csharp
public class YourServiceClientWrapper : IDisposable
{
    private HttpClient _httpClient;

    public void Dispose()
    {
        _httpClient?.Dispose();
    }
}
```

**3. Call Dispose in Provider.Shutdown():**
```csharp
public void Shutdown()
{
    foreach (var client in _clientCache.Values)
    {
        client.Dispose();
    }
    _clientCache.Clear();
}
```

---

## Event Log Reference

### Common Event Log Entries

**Success messages:**
```
Level: Information
Source: BigDrive.Provider.YourService
Message: Provider Startup
Message: EnumerateFolders: path='\folder'
Message: EnumerateFolders: returned 5 folders
```

**Error messages:**
```
Level: Error
Source: BigDrive.Provider.YourService
Message: EnumerateFolders failed: Could not load file or assembly 'YourPackage'
Message: GetFileData failed: The remote server returned an error: (401) Unauthorized
```

### Viewing Event Log in PowerShell

```powershell
# Last 10 entries
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 10

# Errors only
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -EntryType Error -Newest 10

# Real-time monitoring
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 1 -Wait
```

---

## Quick Diagnostic Checklist

When your provider isn't working, run through this checklist:

- [ ] **Built successfully?** Check for compilation errors
- [ ] **Registered?** Check COM+ Applications in Component Services
- [ ] **Running as Admin?** Registration requires elevation
- [ ] **AssemblyResolver.cs exists?** Required for NuGet packages
- [ ] **app.config exists?** Required for version conflicts
- [ ] **Static constructor calls AssemblyResolver.Initialize()?** Required!
- [ ] **CopyLocalLockFileAssemblies enabled?** Check .csproj
- [ ] **All NuGet DLLs in bin folder?** Check bin\Debug\net472\
- [ ] **Check Event Log?** Look for error details
- [ ] **Interface signatures match?** Compare with BigDrive.Interfaces definitions

---

## Getting Help

If you're still stuck after following this guide:

1. **Check Event Log** for detailed error messages (Application log, source "BigDrive.Provider.YourService")
2. **Review existing providers** as working examples (Iso, Archive, Zip, Flickr)
3. **Compare your code** with the templates in [examples.md](examples.md)
4. **Verify prerequisites:**
   - .NET Framework 4.7.2 SDK installed
   - Visual Studio 2022 or later
   - Running as Administrator

---

## See Also

- [Getting Started](getting-started.md) - Project setup
- [Interfaces Reference](interfaces.md) - Interface definitions
- [NuGet Dependencies](nuget-dependencies.md) - AssemblyResolver setup
- [OAuth Authentication](oauth-authentication.md) - OAuth implementation
