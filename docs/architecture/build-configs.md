# BigDrive Build Configurations

This document explains the platform-specific build strategy for BigDrive's mixed C++/C# codebase.

---

## Platform-Specific vs Platform-Agnostic Projects

### C++ Projects: Platform-Specific (x86 and x64)

C++ shell extension projects **must** be built for both 32-bit (x86/Win32) and 64-bit (x64) architectures because they load into `explorer.exe`, which can be either 32-bit or 64-bit depending on the Windows version.

**Projects:**
- BigDrive.Client
- BigDrive.Extension
- BigDrive.ShellFolder
- BigDrive.Client.Test
- BigDrive.ShellFolder.Test

**Why Platform-Specific:**
- Shell extensions are loaded in-process into `explorer.exe`
- DLL must match the bitness of the host process
- Cannot use AnyCPU because .NET components can't load into native C++ processes
- Direct integration with Windows shell APIs that have architecture-specific implementations

### C# Projects: Any CPU

C# components target "Any CPU" and are JIT-compiled to the appropriate architecture at runtime.

**Projects:**
- BigDrive.Shell
- BigDrive.Service
- BigDrive.Provider.* (all providers)
- BigDrive.Interfaces
- BigDrive.Service.Interfaces
- BigDrive.ConfigProvider
- BigDrive.Setup

**Why Any CPU:**
- Run in separate `dllhost.exe` processes via COM+ (out-of-process)
- COM+ handles marshaling across process boundaries regardless of bitness
- No direct dependency on architecture-specific Windows shell APIs
- Simplifies deployment (one binary for all architectures)

---

## Solution Configuration Strategy

The solution uses **blended configurations** to handle the mixed C++/C# architecture. This allows developers to build "Any CPU" for daily work while CI/CD systems can build both x86 and x64 for complete test coverage.

### Configuration Matrix

| Configuration | C++ Projects | C# Projects | Use Case |
|---------------|--------------|-------------|----------|
| **Debug\|Any CPU** | Build as **x64** Debug | Build as **Any CPU** Debug | Daily development (most common) |
| **Release\|Any CPU** | Build as **x64** Release | Build as **Any CPU** Release | Release builds for x64 systems |
| **Debug\|x86** | Build as **Win32** Debug | Build as **Any CPU** Debug | Testing 32-bit Explorer integration |
| **Debug\|x64** | Build as **x64** Debug | Build as **Any CPU** Debug | Testing 64-bit Explorer integration |
| **Release\|x86** | Build as **Win32** Release | Build as **Any CPU** Release | Release builds for x86 systems |
| **Release\|x64** | Build as **x64** Release | Build as **Any CPU** Release | Release builds for x64 systems |

### How Blended Configurations Work

In Visual Studio solution configurations:
- **Solution Platform "Any CPU"** is mapped to:
  - C# projects: Build as "Any CPU"
  - C++ projects: Build as "x64" (default for convenience)

- **Solution Platform "x86"** is mapped to:
  - C# projects: Build as "Any CPU"
  - C++ projects: Build as "Win32"

- **Solution Platform "x64"** is mapped to:
  - C# projects: Build as "Any CPU"
  - C++ projects: Build as "x64"

This allows:
- Developers to use "Debug|Any CPU" for most work
- CI/CD pipelines to build all combinations for complete testing
- Both x86 and x64 shell extensions to be deployed for maximum compatibility

---

## Platform Toolset

### Current Toolset: v145

C++ projects use **Visual Studio 2027 Preview (v145)** platform toolset.

**Project Files:**
```xml
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
  <PlatformToolset>v145</PlatformToolset>
</PropertyGroup>
```

### Upgrading Platform Toolset

When upgrading to a newer Visual Studio version:

1. **Check available toolsets:**
   ```powershell
   Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Recurse -Directory -Filter "MSVC" | 
     Select-Object -First 1 -ExpandProperty FullName | Get-ChildItem | Select-Object Name
   ```

2. **Update all .vcxproj files:**
   ```powershell
   $files = Get-ChildItem -Path "." -Filter "*.vcxproj" -Recurse
   foreach ($file in $files) {
       (Get-Content $file.FullName) -replace '<PlatformToolset>v141</PlatformToolset>', 
                                              '<PlatformToolset>v145</PlatformToolset>' |
       Set-Content $file.FullName -Encoding UTF8
   }
   ```

3. **Update documentation** (this file and `.github/copilot-instructions.md`)

### Toolset Version History

| Toolset | Visual Studio Version |
|---------|----------------------|
| v141 | Visual Studio 2017 |
| v142 | Visual Studio 2019 |
| v143 | Visual Studio 2022 |
| v145 | Visual Studio 2027 Preview |

---

## Target Framework

All C# projects target **.NET Framework 4.7.2**.

**Why .NET Framework (not .NET Core/5+):**
- COM+ ServicedComponent requires .NET Framework
- Mature COM interop support
- Windows-only application (no cross-platform requirement)
- Stability and compatibility with Windows shell APIs

---

## Build Output Directories

### C++ Projects

```
x64/
├── Debug/
│   ├── BigDrive.Client.lib
│   ├── BigDrive.Extension.dll
│   ├── BigDrive.ShellFolder.dll
│   ├── BigDrive.Client.Test.dll
│   └── BigDrive.ShellFolder.Test.dll
└── Release/
    └── (same as Debug)

Win32/  (or x86/)
├── Debug/
│   └── (same structure)
└── Release/
    └── (same structure)
```

### C# Projects

```
src/BigDrive.Shell/bin/
├── Debug/
│   └── net472/
│       └── BigDrive.Shell.exe
└── Release/
    └── net472/
        └── BigDrive.Shell.exe

src/BigDrive.Provider.Flickr/bin/
├── Debug/
│   └── net472/
│       └── BigDrive.Provider.Flickr.dll
└── Release/
    └── net472/
        └── BigDrive.Provider.Flickr.dll
```

---

## Building the Solution

### From Visual Studio

1. **Daily Development:**
   - Select "Debug | Any CPU" configuration
   - Build Solution (Ctrl+Shift+B)
   - C++ projects build as x64, C# projects as Any CPU

2. **Testing x86 Shell Extensions:**
   - Select "Debug | x86" configuration
   - Build Solution
   - Test with 32-bit explorer.exe

3. **Full Build:**
   - Build "Debug | x86"
   - Build "Debug | x64"
   - Ensures all shell extensions work on both architectures

### From Command Line

**Build All Configurations:**
```cmd
msbuild BigDrive.sln /p:Configuration=Debug /p:Platform="Any CPU"
msbuild BigDrive.sln /p:Configuration=Debug /p:Platform=x86
msbuild BigDrive.sln /p:Configuration=Debug /p:Platform=x64
msbuild BigDrive.sln /p:Configuration=Release /p:Platform="Any CPU"
msbuild BigDrive.sln /p:Configuration=Release /p:Platform=x86
msbuild BigDrive.sln /p:Configuration=Release /p:Platform=x64
```

**Build Only C++ Projects:**
```cmd
msbuild BigDrive.sln /p:Configuration=Debug /p:Platform=x64 /t:BigDrive.Client;BigDrive.Extension;BigDrive.ShellFolder
```

**Build Only C# Projects:**
```cmd
msbuild BigDrive.sln /p:Configuration=Debug /p:Platform="Any CPU" /t:BigDrive.Shell;BigDrive.Service
```

---

## CI/CD Considerations

### GitHub Actions / Azure Pipelines

**Strategy:**
1. Build all C# projects once (Any CPU)
2. Build C++ projects twice (x86 and x64)
3. Package both x86 and x64 shell extensions for distribution

**Example Pipeline:**
```yaml
jobs:
  build:
    runs-on: windows-latest
    steps:
      - name: Build C# Components
        run: msbuild BigDrive.sln /p:Configuration=Release /p:Platform="Any CPU"
      
      - name: Build C++ x86
        run: msbuild BigDrive.sln /p:Configuration=Release /p:Platform=x86 /t:BigDrive.Client;BigDrive.Extension;BigDrive.ShellFolder
      
      - name: Build C++ x64
        run: msbuild BigDrive.sln /p:Configuration=Release /p:Platform=x64 /t:BigDrive.Client;BigDrive.Extension;BigDrive.ShellFolder
      
      - name: Package
        run: |
          mkdir package
          copy x64\Release\*.dll package\x64\
          copy Win32\Release\*.dll package\x86\
          copy src\BigDrive.Shell\bin\Release\net472\*.exe package\
```

---

## Project Configuration Best Practices

### When Adding New C++ Projects

1. **Create both Win32 and x64 configurations:**
   ```xml
   <ItemGroup Label="ProjectConfigurations">
     <ProjectConfiguration Include="Debug|Win32">
       <Configuration>Debug</Configuration>
       <Platform>Win32</Platform>
     </ProjectConfiguration>
     <ProjectConfiguration Include="Debug|x64">
       <Configuration>Debug</Configuration>
       <Platform>x64</Platform>
     </ProjectConfiguration>
     <ProjectConfiguration Include="Release|Win32">
       <Configuration>Release</Configuration>
       <Platform>Win32</Platform>
     </ProjectConfiguration>
     <ProjectConfiguration Include="Release|x64">
       <Configuration>Release</Configuration>
       <Platform>x64</Platform>
     </ProjectConfiguration>
   </ItemGroup>
   ```

2. **Set platform toolset for all configurations:**
   ```xml
   <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
     <PlatformToolset>v145</PlatformToolset>
   </PropertyGroup>
   <!-- Repeat for Release|Win32, Debug|x64, Release|x64 -->
   ```

3. **Add to solution with both platforms mapped**

### When Adding New C# Projects

1. **Target "Any CPU":**
   ```xml
   <PropertyGroup>
     <PlatformTarget>AnyCPU</PlatformTarget>
   </PropertyGroup>
   ```

2. **Target .NET Framework 4.7.2:**
   ```xml
   <PropertyGroup>
     <TargetFramework>net472</TargetFramework>
   </PropertyGroup>
   ```

3. **Do not add explicit x86/x64 configurations** (use Any CPU only)

---

## Troubleshooting

### "The build tools for [version] cannot be found"

**Problem:** C++ projects fail to build with platform toolset error.

**Solution:**
1. Check installed toolset version (see "Upgrading Platform Toolset" above)
2. Update all .vcxproj files to use the installed toolset
3. Update documentation

### "Project targets 'AnyCPU' but not available in solution"

**Problem:** C# project can't build because solution configuration doesn't map to "Any CPU".

**Solution:**
1. Open Configuration Manager in Visual Studio
2. Ensure "Any CPU" solution platform exists
3. Map C# projects to "Any CPU" for all solution platforms

### "Shell extension not loading in Explorer"

**Problem:** BigDrive.ShellFolder.dll registered but not appearing in Explorer.

**Solution:**
1. Check if Explorer is 32-bit or 64-bit: Task Manager → Details tab → explorer.exe → Platform column
2. Ensure the matching bitness DLL is registered (x86 for 32-bit, x64 for 64-bit)
3. Re-register: `regsvr32 /u BigDrive.ShellFolder.dll` then `regsvr32 BigDrive.ShellFolder.dll`

---

## See Also

- [Overview](overview.md) — High-level architecture
- [Components](components.md) — Component platform details
- [Installation](installation.md) — Registration and setup
