# Release history

## 0.1.0-b003

### Fixed

- Install the Autotools prerequisites required by pthread-stubs on Linux x64 and ARM64 release runners.

### Added

- Print vcpkg build diagnostics on installation failure and upload per-platform log artifacts.

## 0.1.0-b002

### Fixed

- Fetch the full pinned vcpkg history in the release workflow so historical baselines and raylib 5.5 port trees are available.

## 0.1.0-b001

### Added

- Modular platform, struct, MVVM, algorithm, networking and UI libraries.
- MVVM-bound widget gallery, civil date/time pickers and offscreen raylib rendering.
- Configurable five-area dockable editor workspace.
- Static/shared packaging with module export headers.
- Tag-driven shared-library release archives for Linux, Windows and macOS on x64 and ARM64.
