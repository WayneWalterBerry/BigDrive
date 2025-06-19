# BigDrive

BigDrive is a modular, cross-language framework for building, configuring, and managing virtual drive providers and shell folder integrations on Windows. It supports the development and registration of custom drive providers, configuration management, and seamless integration with Windows shell extensions.

## Features

- **Provider Framework:** Create and register custom drive providers using a unified interface.
- **Configuration Management:** Manage provider and drive configurations, including parsing and storing settings in JSON.
- **Shell Integration:** Implement `IShellFolder` and related interfaces for Windows Explorer integration.
- **Event Logging:** Utilities for writing to the Windows Event Viewer for diagnostics and monitoring.
- **Extensive Unit Tests:** Comprehensive test coverage for all major components.

## Architecture Overview

Windows Shell Folder Extensions are challenging to develop for several reasons:

1. **C++ Implementation:**  
   Shell folder extensions must be implemented in C++. Since they are hosted in `explorer.exe`, loading .NET assemblies is not recommended.
2. **Limited Documentation:**  
   Shell extensions have existed since Windows 95, but early documentation was scarce and not widely available online.
3. **Memory Management:**  
   Extensions must manage memory perfectly, as `explorer.exe` can run for long periods. Even minor leaks can accumulate over time.
4. **Stability:**  
   Access violations in extensions can crash `explorer.exe` and cause the taskbar to disappear, requiring a restart.
5. **COM Complexity:**  
   Developing shell extensions involves implementing numerous COM interfaces and clipboard formats.

To address these challenges, BigDrive uses a C++ shim shell extension that proxies data requests to out-of-process COM+ components. This design ensures:

- **Process Isolation:**  
  COM+ components run outside `explorer.exe`, so crashes do not affect the shell.
- **Language Flexibility:**  
  COM+ components can be written in any language that supports COM on Windows.
- **Extensibility:**  
  Developers can write and register their own COM+ components, which BigDrive will discover and expose as drives in File Explorer under "This PC".

## Installation Architecture

See the [Installation Architecture](docs/architecture/installation.md) for details.

## Project Structure

- `src/BigDrive.Client/` — Core C++ client library for interacting with providers and managing configurations.
- `src/BigDrive.Provider.Sample/` — Example C# provider implementation demonstrating integration with the framework.
- `src/BigDrive.Setup/` — Setup utilities for bootstrapping event logs and managing event sources.
- `src/ConfigProvider/` — Configuration provider logic and models.
- `src/DTC/` — Distributed Transaction Coordinator integration and service logic.
- `src/Interfaces/` — Shared interface definitions for providers and clients.
- `src/IShellFolder/` — Shell extension implementation for Windows Explorer integration.
- `src/Shared/` — Shared utilities and logging components.
- `test/` — Unit tests for all major modules.

## Getting Started

1. **Build the Solution:**  
   Open `BigDrive.sln` in Visual Studio and build all projects.
2. **Run Setup:**  
   Use the setup utilities in `src/BigDrive.Setup/` to register event sources and prepare the environment (requires administrator privileges).
3. **Develop Providers:**  
   Use the sample provider as a template for building your own drive providers.
4. **Testing:**  
   Run the unit tests in the `test/` directory to verify functionality.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.