# NuGet Dependencies and Assembly Resolution

## Critical Requirements Checklist

If your provider uses **any** NuGet packages (libraries, SDKs, etc.), you **MUST** implement ALL of the following:

- [x] **1. Add `AssemblyResolver.cs`** to your project (see template below)
- [x] **2. List all NuGet assemblies** in the `managedAssemblies` array
- [x] **3. Add static constructor** to `Provider.cs` that calls `AssemblyResolver.Initialize()`
- [x] **4. Create `app.config`** with binding redirects for version conflicts
- [x] **5. Add `<CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>`** to `.csproj`

**Missing ANY of these will cause runtime failures during registration or execution!**

---

## Why These Are Required

### The Problem

BigDrive providers run in `dllhost.exe` (COM+ surrogate process), which:

1. ❌ **Does NOT use** `.deps.json` files that .NET Core/5+ rely on
2. ❌ **Does NOT probe** NuGet package cache directories (`~/.nuget/packages/`)
3. ❌ **Does NOT automatically** resolve assembly version conflicts
4. ❌ **Does NOT honor** binding redirects during early type loading

**Result:** Even though your project builds successfully, runtime failures occur when
`regsvcs.exe` or `dllhost.exe` tries to load your provider.

### The Solution

You need **BOTH**:

| Component | Problem It Solves | When It's Used |
|-----------|-------------------|----------------|
| **AssemblyResolver** | Finds assemblies not in standard probe paths | When CLR can't locate a DLL at all |
| **app.config** | Redirects version requests to actual versions | When CLR finds DLL but version doesn't match |
| **Static constructor** | Ensures resolver registers BEFORE type loading | During regsvcs.exe and COM activation |

**Example scenario:**

1. Your provider references `System.Text.Json 9.0.5`
2. ConfigProvider (dependency) was built with `System.Text.Json 9.0.0.11`
3. At runtime, ConfigProvider asks for version 9.0.0.11
4. Your bin folder only has version 9.0.5 (from NuGet restore)

**What happens:**

- **Without AssemblyResolver:** CLR can't find the DLL → `"Could not load file or assembly"`
- **Without app.config:** CLR finds DLL but rejects it → `"manifest definition does not match"`
- **Without static constructor:** Resolver registers too late → `"Exception has been thrown by the target of an invocation"`
- **With ALL THREE:** ✅ CLR uses AssemblyResolver to find the DLL, uses binding redirect to accept version mismatch

---

## 1. Create AssemblyResolver.cs

Add this file to your provider project root:

```csharp
// <copyright file="AssemblyResolver.cs" company="Your Company">
// Copyright (c) Your Company. All rights reserved.
// </copyright>

namespace BigDrive.Provider.YourService
{
    using System;
    using System.IO;
    using System.Reflection;

    /// <summary>
    /// Provides assembly resolution for NuGet package dependencies.
    /// This resolves version conflicts when assemblies are loaded by COM+ (regsvcs/dllhost).
    /// </summary>
    /// <remarks>
    /// The static constructor ensures the resolver is registered before any other code runs,
    /// which is critical for COM+ ServicedComponents where assembly loading happens early.
    /// </remarks>
    internal static class AssemblyResolver
    {
        /// <summary>
        /// Static constructor ensures resolver is registered at type load time,
        /// before any other code in the assembly runs.
        /// </summary>
        static AssemblyResolver()
        {
            AppDomain.CurrentDomain.AssemblyResolve += ResolveAssembly;
        }

        /// <summary>
        /// Initializes the assembly resolver. Call this method early to ensure
        /// the static constructor has run.
        /// </summary>
        public static void Initialize()
        {
            // The static constructor does the work.
            // This method exists to provide an explicit initialization point.
        }

        /// <summary>
        /// Handles assembly resolution by loading assemblies from the same directory
        /// as the executing assembly, ignoring version mismatches.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The event arguments containing the assembly name.</param>
        /// <returns>The resolved assembly, or null to let CLR continue resolution.</returns>
        private static Assembly ResolveAssembly(object sender, ResolveEventArgs args)
        {
            // Parse the requested assembly name
            AssemblyName requestedName = new AssemblyName(args.Name);

            // List of assemblies we handle
            // ADD YOUR PROVIDER'S NUGET DEPENDENCIES HERE!
            string[] managedAssemblies = new string[]
            {
                // Example: System.Text.Json and dependencies (required for IBigDriveDriveInfo)
                "System.Text.Json",
                "System.Runtime.CompilerServices.Unsafe",
                "System.Memory",
                "System.Buffers",
                "System.Threading.Tasks.Extensions",
                "System.Text.Encodings.Web",
                "Microsoft.Bcl.AsyncInterfaces",
                "System.Numerics.Vectors",
                "System.ValueTuple"
            };

            foreach (string assemblyName in managedAssemblies)
            {
                if (requestedName.Name.Equals(assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return TryLoadAssembly(assemblyName);
                }
            }

            return null;
        }

        /// <summary>
        /// Attempts to load an assembly from the executing assembly's directory.
        /// </summary>
        /// <param name="assemblyName">The simple name of the assembly.</param>
        /// <returns>The loaded assembly, or null if not found.</returns>
        private static Assembly TryLoadAssembly(string assemblyName)
        {
            // First, check if it's already loaded
            foreach (Assembly loaded in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (loaded.GetName().Name.Equals(assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return loaded;
                }
            }

            // Try to load from the same directory as this assembly
            string executingPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string assemblyPath = Path.Combine(executingPath, assemblyName + ".dll");

            if (File.Exists(assemblyPath))
            {
                try
                {
                    return Assembly.LoadFrom(assemblyPath);
                }
                catch
                {
                    // Fall through to return null
                }
            }

            return null;
        }
    }
}
```

---

## 2. Add Static Constructor to Provider.cs

In your `Provider.cs` main class, add a static constructor that explicitly initializes AssemblyResolver:

```csharp
public partial class Provider : ServicedComponent,
    IProcessInitializer,
    IBigDriveRegistration,
    IBigDriveEnumerate
{
    private static readonly BigDriveTraceSource DefaultTraceSource = BigDriveTraceSource.Instance;

    /// <summary>
    /// Static constructor to ensure AssemblyResolver is initialized early.
    /// CRITICAL: This must run before any COM+ registration code executes.
    /// </summary>
    static Provider()
    {
        // Force AssemblyResolver static constructor to run
        AssemblyResolver.Initialize();
    }

    // ... rest of your Provider class
}
```

**Why this is required:**

- Static constructors run **before** any static members are accessed
- During `regsvcs.exe` registration, COM+ loads your Provider type before executing methods
- If NuGet dependencies (like System.Text.Json) are needed during type loading, the resolver must already be registered
- Without explicit initialization, the AssemblyResolver may register too late

**Symptoms without static constructor:**

```
Failed to register assembly 'YourProvider, Version=1.0.0.0'
Exception has been thrown by the target of an invocation.
Could not load file or assembly 'System.Text.Json, Version=...'
```

---

## 3. Create app.config with Binding Redirects

Create `app.config` in your provider project root:

```xml
<?xml version="1.0" encoding="utf-8"?>
<configuration>
	<runtime>
		<assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
			<!-- System.Text.Json and dependencies (required for IBigDriveDriveInfo) -->
			<dependentAssembly>
				<assemblyIdentity name="System.Text.Json" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-9.0.0.11" newVersion="9.0.0.11" />
			</dependentAssembly>
			<dependentAssembly>
				<assemblyIdentity name="System.Runtime.CompilerServices.Unsafe" publicKeyToken="b03f5f7f11d50a3a" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-6.0.0.0" newVersion="6.0.0.0" />
			</dependentAssembly>
			<dependentAssembly>
				<assemblyIdentity name="System.Memory" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-4.0.5.0" newVersion="4.0.1.2" />
			</dependentAssembly>
			<dependentAssembly>
				<assemblyIdentity name="System.Buffers" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-4.0.3.0" newVersion="4.0.3.0" />
			</dependentAssembly>
			<dependentAssembly>
				<assemblyIdentity name="System.Threading.Tasks.Extensions" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-4.2.0.1" newVersion="4.2.0.1" />
			</dependentAssembly>
			<dependentAssembly>
				<assemblyIdentity name="System.Text.Encodings.Web" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-9.0.0.0" newVersion="9.0.0.0" />
			</dependentAssembly>
			<dependentAssembly>
				<assemblyIdentity name="Microsoft.Bcl.AsyncInterfaces" publicKeyToken="cc7b13ffcd2ddd51" culture="neutral" />
				<bindingRedirect oldVersion="0.0.0.0-9.0.0.0" newVersion="9.0.0.0" />
			</dependentAssembly>
		</assemblyBinding>
	</runtime>
</configuration>
```

**Why app.config is required:**

- Different project references may request different versions of assemblies
  - Example: ConfigProvider built with `System.Text.Json 9.0.0.11`
  - Example: Your provider references `System.Text.Json 9.0.5`
- Without binding redirects, `regsvcs.exe` fails with version mismatch errors
- Binding redirects tell the CLR: "Use version X for all requests from 0.0.0.0 to Y"

**How to determine correct version numbers:**

1. Build your project successfully
2. Look in `bin\Debug\net472\` folder for actual DLL versions
3. Right-click DLL → Properties → Details tab → File version
4. Use the **highest version present** in your binding redirects
5. Example: If you have `System.Text.Json.dll` version 9.0.0.11, use `newVersion="9.0.0.11"`

**Common error without app.config:**

```
Failed to register assembly 'YourProvider'
Could not load file or assembly 'System.Text.Json, Version=4.0.1.2'
The located assembly's manifest definition does not match the assembly reference. (HRESULT: 0x80131040)
```

**Solution:** Add binding redirect mapping 4.0.1.2 → 9.0.0.11 (or whatever version you have).

---

## 4. Configure .csproj for NuGet

Add these properties to your `.csproj`:

```xml
<PropertyGroup>
  <CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
</PropertyGroup>
```

**What this does:**

- Copies **all** NuGet package DLLs (including transitive dependencies) to your `bin\` folder
- Without this, only direct dependencies are copied
- AssemblyResolver can't load assemblies that aren't in the bin folder

---

## Examples by Provider Type

### Archive Provider (SharpCompress + System.Text.Json)

**managedAssemblies:**

```csharp
string[] managedAssemblies = new string[]
{
    // SharpCompress and its dependencies
    "SharpCompress",
    // System.Text.Json for IBigDriveDriveInfo.GetDriveParameters()
    "System.Text.Json",
    "System.Runtime.CompilerServices.Unsafe",
    "System.Memory",
    "System.Buffers",
    "System.Threading.Tasks.Extensions",
    "System.Text.Encodings.Web",
    "Microsoft.Bcl.AsyncInterfaces",
    "System.Numerics.Vectors"
};
```

**PackageReferences:**

```xml
<ItemGroup>
  <PackageReference Include="SharpCompress" Version="0.37.2" />
  <PackageReference Include="System.Text.Json" Version="9.0.5" />
</ItemGroup>
```

---

### ISO Provider (DiscUtils + System.Text.Json)

**managedAssemblies:**

```csharp
string[] managedAssemblies = new string[]
{
    // DiscUtils and its dependencies
    "DiscUtils.Core",
    "DiscUtils.Streams",
    "DiscUtils.Iso9660",
    // System.Text.Json for IBigDriveDriveInfo.GetDriveParameters()
    "System.Text.Json",
    "System.Runtime.CompilerServices.Unsafe",
    "System.Memory",
    "System.Buffers",
    "System.Threading.Tasks.Extensions",
    "System.Text.Encodings.Web",
    "Microsoft.Bcl.AsyncInterfaces",
    "System.Numerics.Vectors",
    "System.ValueTuple"
};
```

**PackageReferences:**

```xml
<ItemGroup>
  <PackageReference Include="DiscUtils.Iso9660" Version="0.16.13" />
  <PackageReference Include="System.Text.Json" Version="9.0.5" />
</ItemGroup>
```

---

### Cloud Provider (Azure.Storage.Blobs)

**managedAssemblies:**

```csharp
string[] managedAssemblies = new string[]
{
    // Azure SDK dependencies
    "Azure.Core",
    "Azure.Storage.Blobs",
    "Azure.Storage.Common",
    "Azure.Identity",
    // System.Text.Json (Azure SDK uses it)
    "System.Text.Json",
    "System.Runtime.CompilerServices.Unsafe",
    "System.Memory",
    "System.Buffers",
    "System.Threading.Tasks.Extensions",
    "System.Text.Encodings.Web",
    "Microsoft.Bcl.AsyncInterfaces",
    "System.Numerics.Vectors"
};
```

**PackageReferences:**

```xml
<ItemGroup>
  <PackageReference Include="Azure.Storage.Blobs" Version="12.x.x" />
  <PackageReference Include="System.Text.Json" Version="9.0.5" />
</ItemGroup>
```

---

## How to Find Required Assemblies

Follow these steps to discover which assemblies to add:

### Step 1: Build and Check bin Folder

```powershell
# Build your project
msbuild YourProvider.csproj /p:Configuration=Debug

# List all DLLs in output directory
dir bin\Debug\net472\*.dll
```

Look for:
- Your NuGet package DLLs (e.g., `SharpCompress.dll`, `DiscUtils.Core.dll`)
- Dependencies (e.g., `System.Text.Json.dll`, `System.Memory.dll`)

### Step 2: Attempt Registration

```powershell
# Run as Administrator
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "bin\Debug\net472\YourProvider.dll"
```

### Step 3: Check Event Log for Failures

If registration fails:

1. Open **Event Viewer** (eventvwr.msc)
2. Navigate to **Windows Logs** → **Application**
3. Look for errors from source **"BigDrive.Provider.YourService"**
4. Find errors like:
   ```
   Could not load file or assembly 'MissingAssembly, Version=1.2.3.4' or one of its dependencies.
   ```

### Step 4: Add Missing Assemblies

Add each missing assembly name to the `managedAssemblies` array in `AssemblyResolver.cs`.

### Step 5: Rebuild and Test

Rebuild and try registering again. Repeat until registration succeeds.

---

## Common Dependencies by Interface

### Always Required (if using ConfigProvider)

These are needed by BigDrive infrastructure:

```csharp
"System.Text.Json",
"System.Runtime.CompilerServices.Unsafe",
"System.Memory",
"System.Buffers"
```

### IBigDriveDriveInfo (GetDriveParameters)

Requires System.Text.Json for serializing parameter definitions:

```csharp
"System.Text.Json",
"System.Runtime.CompilerServices.Unsafe",
"System.Memory",
"System.Buffers",
"System.Threading.Tasks.Extensions",
"System.Text.Encodings.Web",
"Microsoft.Bcl.AsyncInterfaces"
```

### HTTP Clients (HttpClient, RestSharp, Flurl)

```csharp
"System.Net.Http",
"System.Threading.Tasks.Extensions"
```

### Azure SDK (Azure.Storage, Azure.Identity)

```csharp
"Azure.Core",
"Azure.Storage.Blobs",  // or other Azure.Storage.* packages
"Azure.Storage.Common",
"Azure.Identity",
"System.Text.Json",
"System.Memory",
"System.Buffers",
"System.Threading.Tasks.Extensions"
```

### Newtonsoft.Json (if using)

```csharp
"Newtonsoft.Json"
```

---

## Testing Assembly Resolution

### Test 1: Registration

```powershell
# Run as Administrator
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regsvcs.exe "YourProvider.dll"

# Should output:
# Installed Assembly:
#   Assembly: YourProvider.dll
#   Application: BigDrive.Provider.YourService
#   TypeLib: YourProvider.tlb
```

### Test 2: Event Log Check

```powershell
# Check for errors in Event Log
Get-EventLog -LogName Application -Source "BigDrive.Provider.YourService" -Newest 10
```

Look for:
- ✅ "Provider Startup" messages
- ✅ "Register: Provider registered successfully"
- ❌ "Could not load file or assembly" errors

### Test 3: Runtime Execution

```sh
# Create a drive using your provider
bigdrive drive create --provider YourService --name "Test"

# Try listing files (triggers COM activation)
bigdrive ls

# Check for errors
bigdrive status
```

If you see "Could not load file or assembly" during runtime:
1. Check Windows Event Log for the missing assembly name
2. Add it to `managedAssemblies` in AssemblyResolver.cs
3. Rebuild and re-register

---

## Binding Redirect Version Reference

Use these version numbers based on the package version you installed:

| Package | NuGet Version | Assembly Version | Binding Redirect newVersion |
|---------|---------------|------------------|----------------------------|
| System.Text.Json | 4.7.2 | 4.0.1.2 | 4.0.1.2 |
| System.Text.Json | 9.0.5 | 9.0.0.5 | 9.0.0.5 |
| System.Text.Json | 9.0.11 | 9.0.0.11 | 9.0.0.11 |
| System.Memory | 4.5.5 | 4.0.1.2 | 4.0.1.2 |
| System.Buffers | 4.5.1 | 4.0.3.0 | 4.0.3.0 |
| System.Runtime.CompilerServices.Unsafe | 6.0.0 | 6.0.0.0 | 6.0.0.0 |

**Best practice:** Use the version number from the actual DLL in your bin folder (check Properties → Details → File version).

---

## Troubleshooting

### Error: "Could not load file or assembly 'X, Version=Y'"

**Problem:** Assembly is missing from AssemblyResolver OR version mismatch in app.config.

**Solution:**
1. Check if DLL exists in `bin\Debug\net472\` folder
2. If missing: Add to `.csproj` as PackageReference
3. If present: Add assembly name to `managedAssemblies` in AssemblyResolver.cs
4. If version mismatch: Update binding redirect in app.config

### Error: "The located assembly's manifest definition does not match"

**Problem:** Binding redirect is incorrect or missing.

**Solution:**
1. Check actual DLL version in bin folder (Properties → Details)
2. Update `newVersion` in app.config binding redirect to match actual version
3. Rebuild

### Error: AssemblyResolver doesn't seem to run

**Problem:** Static constructor not called early enough.

**Solution:**
1. Verify `static Provider()` constructor exists and calls `AssemblyResolver.Initialize()`
2. Ensure AssemblyResolver.cs is included in the project
3. Clean and rebuild

### Warning: "Found conflicts between different versions"

**This is normal** and expected! The warning means MSBuild detected version conflicts
but resolved them. As long as:
- ✅ AssemblyResolver is implemented
- ✅ app.config has binding redirects
- ✅ Static constructor initializes the resolver

Then the warnings are safe to ignore.

---

## See Also

- [Getting Started](getting-started.md) - Project setup
- [Troubleshooting](troubleshooting.md) - Common errors and solutions
- [ISO Provider](../../src/BigDrive.Provider.Iso/) - Complete example with DiscUtils
- [Archive Provider](../../src/BigDrive.Provider.Archive/) - Complete example with SharpCompress
