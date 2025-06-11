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