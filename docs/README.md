# BigDrive Documentation

Welcome to the BigDrive documentation! This is your starting point for understanding, using, and extending BigDrive.

---

## 📚 Documentation Overview

BigDrive documentation is organized by audience and purpose:

### 👤 **For Users**

| Document | Description |
|----------|-------------|
| **[BigDrive Shell User Guide](BigDrive.Shell.UserGuide.md)** | Complete guide to using BigDrive shell commands |
| **[Flickr Provider Quick Start](FlickrProviderQuickStart.md)** | Quick setup guide for connecting Flickr accounts |

### 👨‍💻 **For Developers**

| Document | Description |
|----------|-------------|
| **[Provider Development](provider-development/README.md)** | **START HERE** - Complete guide to creating BigDrive providers |
| **[Architecture Documentation](architecture/)** | System architecture, security model, installation flow |
| **[Usage Scenarios](scenarios/)** | Real-world use cases and examples |

---

## Quick Links

### I Want To...

#### **Use BigDrive**
- 📖 [Learn shell commands](BigDrive.Shell.UserGuide.md) - `dir`, `copy`, `cd`, etc.
- 🔗 [Connect Flickr account](FlickrProviderQuickStart.md) - OAuth setup

#### **Create a Provider**
- 🚀 **[→ Provider Development Guide](provider-development/README.md)** - Complete developer documentation
  - [Getting Started](provider-development/getting-started.md) - Project setup
  - [Full Development Guide](provider-development/guide.md) - Comprehensive reference
  - [Development Practices](provider-development/practices.md) - Build-register-test workflow
  - [Interfaces Reference](provider-development/interfaces.md) - Interface definitions
  - [NuGet Dependencies](provider-development/nuget-dependencies.md) - **CRITICAL!** AssemblyResolver setup
  - [OAuth Authentication](provider-development/oauth-authentication.md) - Cloud service auth
  - [Troubleshooting](provider-development/troubleshooting.md) - Common errors

#### **Understand Architecture**
- 🏛️ [Architecture Overview](architecture/) - System design and components
- 🔐 [Security Model](architecture/) - COM+ identity and permissions
- 📦 [Installation Flow](architecture/) - Setup and registration process

---

## Documentation by Provider Type

### Local File Providers (ISO, VHD, Archives)

**Read:** [Provider Development → Getting Started](provider-development/getting-started.md)

**Study these providers:**
- `src/BigDrive.Provider.Iso/` - Simplest read-only example
- `src/BigDrive.Provider.VirtualDisk/` - Read-write VM disk images
- `src/BigDrive.Provider.Archive/` - Multi-format archives

### Cloud API Providers (OneDrive, Google Drive, Flickr)

**Read:** [Provider Development → OAuth Authentication](provider-development/oauth-authentication.md)

**Study these providers:**
- `src/BigDrive.Provider.Flickr/` - OAuth 1.0a reference implementation

---

## Common Tasks

### Create a New Provider

1. [Project Setup](provider-development/getting-started.md)
2. [Implement Interfaces](provider-development/interfaces.md)
3. [Setup NuGet Dependencies](provider-development/nuget-dependencies.md) (if using packages)
4. [Build and Test](provider-development/practices.md)

### Add OAuth to Provider

1. [OAuth Authentication Guide](provider-development/oauth-authentication.md)
2. Implement `IBigDriveAuthentication`
3. Handle `BigDriveAuthenticationRequiredException`
4. Test with `bigdrive login` command

### Debug Provider Issues

1. Check [Troubleshooting Guide](provider-development/troubleshooting.md)
2. Review Windows Event Log: `Get-EventLog -LogName Application -Source "BigDrive.Provider.YourName" -Newest 20`
3. Follow [Development Practices](provider-development/practices.md) for debugging workflow

---

## Provider Development Checklist

Quick checklist for provider readiness:

### Core Implementation
- [ ] Unique CLSID (GUID)
- [ ] Implements `IProcessInitializer`
- [ ] Implements `IBigDriveRegistration` with `[ComRegisterFunction]`
- [ ] Implements `IBigDriveEnumerate`
- [ ] Static constructor calls `AssemblyResolver.Initialize()`

### NuGet Dependencies (if applicable)
- [ ] `AssemblyResolver.cs` with all NuGet assembly names
- [ ] `app.config` with binding redirects
- [ ] `<CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>` in .csproj

### Documentation
- [ ] `README.md` in provider project root
- [ ] XML doc comments on all public methods
- [ ] File copyright headers

### Testing
- [ ] Builds successfully
- [ ] Registers with `regsvcs.exe`
- [ ] Listed in `bigdrive providers`
- [ ] Can create drive and enumerate files

**Full checklist:** See [Development Guide](provider-development/guide.md#checklist)

---

## Existing Providers Reference

Study these working implementations:

| Provider | Path | Complexity | Read | Write | OAuth |
|----------|------|------------|------|-------|-------|
| **VirtualDisk** | `src/BigDrive.Provider.VirtualDisk/` | ⭐⭐⭐ Medium | ✅ | ✅ | ❌ |
| **Iso** | `src/BigDrive.Provider.Iso/` | ⭐⭐ Simple | ✅ | ❌ | ❌ |
| **Archive** | `src/BigDrive.Provider.Archive/` | ⭐⭐⭐ Medium | ✅ | ❌ | ❌ |
| **Zip** | `src/BigDrive.Provider.Zip/` | ⭐⭐ Simple | ✅ | ✅ | ❌ |
| **Flickr** | `src/BigDrive.Provider.Flickr/` | ⭐⭐⭐⭐ Complex | ✅ | ❌ | ✅ |

**Recommendation:**
- Learning providers? → Start with **Iso** (simplest)
- Need read-write? → Study **VirtualDisk** or **Zip**
- Need OAuth? → Study **Flickr**

---

## Architecture Quick Reference

```
BigDrive.Shell / Windows Explorer
        │
        │ COM Activation
        ▼
    dllhost.exe (Interactive User)
        │
        ├─► Your.Provider.dll
        │       │
        │       └─► Your API/SDK
        │
        └─► External Storage (Cloud, Database, Files)
```

**Key Points:**
- Providers run **out-of-process** (not in explorer.exe)
- Providers run as **Interactive User** (logged-in user)
- Access to Credential Manager for secrets
- Isolated from Explorer crashes

**Details:** [Architecture Overview](architecture/)

---

## Documentation Organization

```
docs/
├── README.md (this file)                   # Root table of contents
├── BigDrive.Shell.UserGuide.md             # End-user shell commands
├── FlickrProviderQuickStart.md             # Flickr setup guide
│
├── provider-development/                   # Provider developer docs
│   ├── README.md                           # Provider development overview
│   ├── getting-started.md                  # Project setup guide
│   ├── guide.md                            # Complete reference guide
│   ├── practices.md                        # Build-register-test workflow
│   ├── interfaces.md                       # Interface reference
│   ├── nuget-dependencies.md               # AssemblyResolver (CRITICAL!)
│   ├── oauth-authentication.md             # OAuth implementation
│   └── troubleshooting.md                  # Common errors
│
├── architecture/                           # System architecture
│   ├── overview.md                         # Architecture diagrams
│   ├── installation.md                     # Setup and registration
│   └── security.md                         # Security model
│
└── scenarios/                              # Use case examples
    └── (usage scenarios)
```

---

## Contributing to Documentation

Found an issue or want to improve the docs?

1. **File an issue:** https://github.com/WayneWalterBerry/BigDrive/issues
2. **Submit a pull request** with improvements
3. **Share your provider** as a reference implementation

### Documentation Standards

- Use Markdown for all documentation files
- Include code examples with syntax highlighting
- Add ASCII diagrams for architecture (works in terminals)
- Cross-link related documents
- Keep project READMEs developer-focused
- Keep `docs/` files user-focused

---

## Need Help?

- 🔍 **Provider not working?** → [Troubleshooting Guide](provider-development/troubleshooting.md)
- 📖 **Need code examples?** → Open existing provider source code
- ❓ **Have questions?** → File an issue on GitHub
- 💡 **Want to contribute?** → See Contributing section above

---

**Happy building!** 🚀

*Copyright © Wayne Walter Berry. All rights reserved.*
