# BigDrive

BigDrive is a modular, cross-language framework for building, configuring, and managing virtual drive providers and shell folder integrations on Windows. It is designed to support the development and registration of custom drive providers, facilitate configuration management, and enable integration with Windows shell extensions.

## Features
- **Provider Framework:** Create and register custom drive providers using a unified interface.
- **Configuration Management:** Manage provider and drive configurations, including parsing and storing settings in JSON.
- **Shell Integration:** Support for implementing IShellFolder and related interfaces for Windows Explorer integration.
- **Event Logging:** Utilities for writing to the Windows Event Viewer for diagnostics and monitoring.
- **Extensive Unit Tests:** Comprehensive test coverage for all major components.

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
1. **Build the Solution:** Open `BigDrive.sln` in Visual Studio and build all projects.
2. **Run Setup:** Use the setup utilities in `src/BigDrive.Setup/` to register event sources and prepare the environment (requires administrator privileges).
3. **Develop Providers:** Use the sample provider as a template for building your own drive providers.
4. **Testing:** Run the unit tests in the `test/` directory to verify functionality.

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.