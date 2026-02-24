# BigDrive Provider Development Practices

This document outlines standard development practices for creating and debugging BigDrive providers. It covers the build-register-test cycle, common issues, and troubleshooting techniques.

---

## Prerequisites

Before developing a provider, ensure:

1. **Visual Studio runs as Administrator** — Required for `regsvcs.exe` to register COM+ components
2. **BigDrive.Setup.exe has been run** — Creates registry keys and service account
3. **Component Services is accessible** — `comexp.msc` or `dcomcnfg.exe`

---

## Development Cycle

### Build → Register → Test → Repeat

```
┌─────────────────────────────────────────────────────────────────────────┐
│                      DEVELOPMENT CYCLE                                  │
│                                                                         │
│   1. Edit Code                                                          │
│         │                                                               │
│         ▼                                                               │
│   2. Build (Ctrl+Shift+B)                                               │
│         │                                                               │
│         ├──► PreBuild: regsvcs.exe /u Provider.dll (unregister)         │
│         │                                                               │
│         ├──► Compile                                                    │
│         │                                                               │
│         └──► PostBuild: regsvcs.exe Provider.dll (register)             │
│                   │                                                     │
│                   ├──► Creates COM+ Application                         │
│                   ├──► Calls [ComRegisterFunction]                      │
│                   │       ├─► SetApplicationIdentityToInteractiveUser() │
│                   │       └─► Provider.Register()                       │
│                   │                                                     │
│         ▼                                                               │
│   3. Test in BigDrive.Shell or Explorer                                 │
│         │                                                               │
│         ▼                                                               │
│   4. Debug (if needed)                                                  │
│         │                                                               │
│         └──► STOP COM+ APPLICATION before next build!                   │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## The DLL Locking Problem

### Why Builds Fail After Testing

When you test your provider, COM+ loads your DLL into `dllhost.exe`. This process **locks the DLL file**, preventing the next build from overwriting it.

```
Build Error:
  error MSB3021: Unable to copy file "bin\Debug\Provider.dll" 
  to "bin\Debug\Provider.dll". The process cannot access the file 
  because it is being used by another process.
```

### Solution: Stop the COM+ Application

**Option 1: Component Services (GUI)**

1. Open `comexp.msc` (Component Services)
2. Navigate to: `Component Services → Computers → My Computer → COM+ Applications`
3. Find your provider: `BigDrive.Provider.Flickr` or `BigDrive.Provider.Sample`
4. Right-click → **Shut Down**

**Option 2: PowerShell (Quick)**

```powershell
# Stop a specific COM+ application
$comAdmin = New-Object -ComObject COMAdmin.COMAdminCatalog
$apps = $comAdmin.GetCollection("Applications")
$apps.Populate()
foreach ($app in $apps) {
    if ($app.Name -eq "BigDrive.Provider.Flickr") {
        $comAdmin.ShutdownApplication("BigDrive.Provider.Flickr")
        Write-Host "Stopped BigDrive.Provider.Flickr"
    }
}
```

**Option 3: Kill dllhost.exe (Nuclear Option)**

```powershell
# WARNING: This kills ALL COM+ surrogate processes
Get-Process dllhost -ErrorAction SilentlyContinue | Stop-Process -Force
```

**Option 4: Task Manager**

1. Open Task Manager
2. Find `dllhost.exe` processes (may be multiple)
3. End task on each one

---

## Running Visual Studio Elevated

The PostBuild step (`regsvcs.exe`) requires Administrator privileges to:
- Write to `HKEY_CLASSES_ROOT\CLSID`
- Create/modify COM+ applications
- Write to `HKEY_LOCAL_MACHINE\SOFTWARE\BigDrive`

### To Run Visual Studio as Administrator:

1. Right-click Visual Studio shortcut
2. Select "Run as administrator"
3. Verify title bar shows: `Microsoft Visual Studio (Administrator)`

### If You Forget to Elevate:

```
PostBuild Error:
  RegSvcs : error RA0000 : An error occurred while processing installation
  Access is denied.
```

---

## Debugging Providers

### Attach to dllhost.exe

1. Set a breakpoint in your provider code
2. In Visual Studio: `Debug → Attach to Process`
3. Check "Show processes from all users"
4. Find `dllhost.exe` (there may be several)
5. Select the one hosting your provider and click "Attach"

**Tip:** Add `System.Diagnostics.Debugger.Launch()` temporarily to force the debugger to attach:

```csharp
public string[] EnumerateFolders(Guid driveGuid, string path)
{
    System.Diagnostics.Debugger.Launch();  // Opens JIT debugger dialog
    // ... rest of code
}
```

### Enable Tracing

Providers use `BigDriveTraceSource` for logging. View logs in:
- **Event Viewer:** `Windows Logs → Application` (source: BigDrive.Provider.Flickr)
- **DebugView:** Download from Sysinternals, run elevated

---

## Project Configuration

### Required Assembly Attributes

```csharp
// Properties/AssemblyInfo.cs

[assembly: ComVisible(true)]
[assembly: Guid("YOUR-UNIQUE-GUID-HERE")]

// COM+ Server activation (out-of-process in dllhost.exe)
[assembly: ApplicationActivation(ActivationOption.Server)]
[assembly: ApplicationAccessControl(false)]
```

### Required Project References

```xml
<!-- .csproj file -->
<ItemGroup>
  <Reference Include="Microsoft.CSharp" />           <!-- For 'dynamic' keyword -->
  <Reference Include="System.Configuration.Install" />
  <Reference Include="System.EnterpriseServices" />
</ItemGroup>

<ItemGroup>
  <ProjectReference Include="..\ConfigProvider\BigDrive.ConfigProvider.csproj" />
  <ProjectReference Include="..\Interfaces\BigDrive.Interfaces.csproj" />
</ItemGroup>
```

### Build Events

```xml
<!-- Unregister before build (in case DLL is locked) -->
<Target Name="PreBuild" BeforeTargets="PreBuildEvent">
  <Exec Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u &quot;$(TargetPath)&quot; || exit /b 0" />
</Target>

<!-- Register after build -->
<Target Name="PostBuild" AfterTargets="PostBuildEvent">
  <Exec Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe &quot;$(TargetPath)&quot;" />
</Target>
```

---

## Common Issues and Solutions

### Issue: "Access to registry key is denied"

**Cause:** The BigDrive registry keys don't grant write access to Administrators.

**Solution:** Run `BigDrive.Setup.exe` as Administrator to create keys with proper permissions.

---

### Issue: "COM+ application not found" during SetApplicationIdentityToInteractiveUser

**Cause:** `regsvcs.exe` creates the COM+ app AFTER calling `[ComRegisterFunction]`, so the app doesn't exist yet.

**Solution:** This is expected on first registration. The identity will be set on subsequent builds. Alternatively, restructure to set identity after `regsvcs.exe` completes.

---

### Issue: Build succeeds but provider doesn't appear in `providers` command

**Cause:** `Provider.Register()` failed silently or threw an exception.

**Solution:** 
1. Check Event Viewer for errors
2. Manually run `regsvcs.exe` from elevated command prompt to see full error output:
   ```cmd
   C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "path\to\Provider.dll"
   ```

---

### Issue: "The system cannot find the file specified" when testing

**Cause:** Provider DLL or dependencies not in the correct location.

**Solution:** Ensure all referenced DLLs are in the same directory as the provider DLL.

---

### Issue: Changes don't take effect after rebuild

**Cause:** Old version of DLL still loaded in `dllhost.exe`.

**Solution:** Stop the COM+ application (see "The DLL Locking Problem" above).

---

## Quick Reference Commands

### PowerShell: Stop All BigDrive COM+ Applications

```powershell
$comAdmin = New-Object -ComObject COMAdmin.COMAdminCatalog
@("BigDrive.Provider.Flickr", "BigDrive.Provider.Sample", "BigDrive.Service") | ForEach-Object {
    try {
        $comAdmin.ShutdownApplication($_)
        Write-Host "Stopped: $_"
    } catch {
        Write-Host "Not running or not found: $_"
    }
}
```

### PowerShell: List All COM+ Applications

```powershell
$comAdmin = New-Object -ComObject COMAdmin.COMAdminCatalog
$apps = $comAdmin.GetCollection("Applications")
$apps.Populate()
$apps | ForEach-Object { Write-Host $_.Name }
```

### Command Prompt: Manual Registration

```cmd
:: Register (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "C:\path\to\Provider.dll"

:: Unregister (elevated)
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe /u "C:\path\to\Provider.dll"
```

### BigDrive Shell: Verify Provider Registration

```
BD> providers

 Registered Providers:

    BigDrive.Provider.Flickr
    CLSID: {B3D8F2A1-7C4E-4A9B-8E1F-2C3D4E5F6A7B}

    BigDrive.Provider.Sample
    CLSID: {F8FE2E5A-E8B8-4207-BC04-EA4BCD4C4361}

    2 Provider(s)
```

---

## Recommended Development Workflow

1. **Start of day:**
   - Launch Visual Studio as Administrator
   - Stop any running COM+ applications from previous session

2. **Before each build:**
   - If you've tested recently, stop the COM+ application
   - Or use the PreBuild event (already configured)

3. **After build:**
   - Check Output window for regsvcs errors
   - If successful, test in BigDrive.Shell

4. **Debugging:**
   - Add `Debugger.Launch()` for quick attach
   - Or manually attach to `dllhost.exe`

5. **End of day:**
   - Stop COM+ applications to release locks
   - This allows other tools (antivirus, backup) to access files

---

## See Also

- [ProviderDevelopmentGuide.md](../ProviderDevelopmentGuide.md) — Full provider implementation guide
- [architecture/security.md](architecture/security.md) — Security model and identity details
- [BigDrive.Setup/README.txt](../../src/BigDrive.Setup/README.txt) — Setup and registration architecture
