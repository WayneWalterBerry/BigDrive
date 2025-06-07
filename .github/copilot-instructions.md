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
  - `<summary>` in `.h` declarations, including `<params>` and `<returns>`.
  - `/// <inheritdoc>` in `.cpp` implementations.
  - For methods implementing shell folder extensions interface methods, detailed comments on inputs, outputs, and implementation reasoning.
- Global Calls: Prefix all global method calls with `::`.
- Precompiled Header: Include `#include "pch.h"` in `.cpp`.
- File Generation: Create Both .h and .cpp files.

#### C# Code
- Comments:
  - `<summary>` for each method, including `<params>` and `<returns>`.
  - For shell folder extensions, detailed comments on inputs, outputs, and implementation reasoning.
- Namespace Usage: Place `using` statements inside the namespace.

#### General Preferences
- File Header:
  ```cpp
  // <copyright file="FileName.cpp" company="Wayne Walter Berry">
  // Copyright (c) Wayne Walter Berry. All rights reserved.
  // </copyright>
- ```