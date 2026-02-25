### Before Making Changes

> **IMPORTANT: Read the Project README First**
>
> Before making any code changes to a project, **read and understand the project's README.md** 
> (or README.txt). Each project has a README that explains its architecture, design principles,
> and constraints that must be followed.

#### Why This Matters

- **BigDrive.Shell**: Uses out-of-process COM+ activation. Provider assemblies must NEVER be 
  loaded into the Shell process. See `src/BigDrive.Shell/README.md`.

- **Provider Projects**: Run in `dllhost.exe` via COM+. Must handle assembly resolution for 
  NuGet dependencies. See individual provider READMEs.

- **BigDrive.Interfaces**: Contains COM interface definitions. Changes affect all providers 
  and clients. Must maintain COM compatibility.

Failing to understand these architectural constraints will result in broken code.

---

### Coding Style Preferences

#### C++ Code
- Error Handling: Use `FAILED()` checks for `HRESULT` and `goto End:` for cleanup.
- Variable Management:
  - Initialize all local variables at method start (`nullptr` or similar).
  - Reset cleaned-up variables to `nullptr`.
- Formatting:
  - Use `{}` with every `if`, with `{` on a new line.
  - CR/LF line endings.
- Comments:
  - member variables of a class should have summary comments in the header file.
  - `<summary>` in `.h` declarations, including `<params>` and `<returns>`.
  - `/// <inheritdoc>` in `.cpp` implementations.
  - For methods implementing interface methods (i.e. override), create detailed summary comments on inputs, outputs, and implementation reasoning in the .h file only.
- Global Calls: Prefix all free (non-member) global function calls with ::. Do not use the :: prefix for static class method calls (e.g., use BigDriveTraceLogger::LogEnter(...)).
- Precompiled Header: Include `#include "pch.h"` in `.cpp`.
- File Generation: Create Both .h and .cpp files.
- Don't use Standard Library functions or Standard Template Library (STL) functions.
- Declare HRESULT variables as `HRESULT hr = S_OK;`.
- private class members should be prefixed with m_.
- Put all private member variables at the top of the class in a private: section, before any methods, and prefix them with m_.
- private methods should be at the bottom of the class declaration
- All method parameters should be passed by reference if possible (using `&` for C++).
- When writing a new method return HRESULT unless there is a specific reason not to, pass the return value back through a method parameter passed by reference.
- put an extra CR/LF at the after `End:`
- after `private:` and `public:` put an extra CR/LF
- Use STDMETHODIMP instead of IFACEMETHODIMP for methods that implement interface methods.

#### C# Code
- Comments:
  - `<summary>` for each method, including `<params>` and `<returns>`.
  - `<summary>` for each class and interface.
  - standard `<summary>` constructors.
- Namespace Usage: Place `using` statements inside the namespace.
- Learn how to use partial classes and recognize that they might need additional file references.

#### General Preferences
- File Header:
  ```cpp
  // <copyright file="FileName.cpp" company="Wayne Walter Berry">
  // Copyright (c) Wayne Walter Berry. All rights reserved.
  // </copyright>
  ```

---

### Documentation Maintenance

When making code changes, keep documentation in sync:

#### Project README Files
Each project has a `README.md` or `README.txt` for **developers/contributors**:
- `src/BigDrive.Shell/README.md` — Shell architecture, **out-of-process COM+ activation**, provider isolation
- `src/BigDrive.Provider.Flickr/README.txt` — Provider implementation details
- `src/ConfigProvider/README.txt` — Registry structure, API reference
- `src/Interfaces/README.txt` — Interface definitions
- `src/BigDrive.Setup/README.txt` — Installation internals

**Read BEFORE coding:**
- Understand the project's architecture and constraints
- Know what dependencies are allowed
- Understand process isolation boundaries (Shell vs Provider)

**Update when:**
- Adding/removing/renaming files in the project
- Changing public APIs or method signatures
- Modifying architecture or data flow
- Adding new dependencies

#### User Documentation (`docs/` directory)
User-facing guides for **end users and contributors**:
- `docs/BigDrive.Shell.UserGuide.md` — Shell commands for users
- `docs/ProviderDevelopmentGuide.md` — How to create new providers
- `docs/architecture/overview.md` — System architecture diagrams
- `docs/architecture/installation.md` — Setup and registration flow

**Update when:**
- Adding/removing shell commands
- Changing command syntax or behavior
- Adding new interfaces or provider requirements
- Modifying registry structure or installation process

#### Key Concepts to Document
- **Providers vs Drives**: Providers define *how* to access storage; Drives are user instances
- **Registry locations**: `SOFTWARE\BigDrive\Providers\` and `SOFTWARE\BigDrive\Drives\`
- **COM+ activation**: Providers run out-of-process in `dllhost.exe`
- **Partial classes**: One file per interface (e.g., `Provider.IBigDriveEnumerate.cs`)

#### Documentation Style
- Use ASCII diagrams for architecture (works in terminals and GitHub)
- Include code examples with proper syntax highlighting
- Keep project READMEs developer-focused (no user tutorials)
- Keep `docs/` files user-focused (commands, guides, tutorials)
- Add "See Also" sections linking related documentation