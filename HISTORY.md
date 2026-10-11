# Release history

## 0.1.0-b005

### Added

- Independent `concurrency` module, available through the `concurrency` vcpkg feature and `cpptoolkit::concurrency` CMake target, with static/shared builds and Windows export headers.
- Thread-safe cooperative cancellation through `CancellationSource` and copyable `CancellationToken` handles sharing cancellation state.
- TestUI Feedback example demonstrating background task progress, cancellation, restart and safe worker shutdown.
- Regression tests for cancellation, dispatcher snapshot ordering and FIFO background execution.
- Security policy documenting supported versions and vulnerability reporting.

### Changed (breaking changes)

- Move `Dispatcher` and `NotificationWorker` from `cpptoolkit::mvvm` to `cpptoolkit::concurrency`. Replace their `cpptoolkit/mvvm/` includes with `cpptoolkit/concurrency/` includes, or use `cpptoolkit/concurrency/concurrency.h`; they are no longer included by `mvvm.h`.
- Make `mvvm` header-only: no MVVM DLL/shared/static library or generated `mvvm/Export.h` is produced. The `cpptoolkit::mvvm` target remains available.
- Make UI depend on `concurrency` in CMake and vcpkg. Non-UI consumers of the moved queues must explicitly enable and link `concurrency`. Rebuild consumers against the updated package.
- Update release packaging to include the compiled concurrency library instead of the former MVVM library. `Application::Dispatch()` and `RunAsync()` retain their existing execution behavior.

### Fixed

- Configure Linux IntelliSense with GCC and the project compilation database to avoid false override errors caused by obsolete header copies.

## 0.1.0-b004

### Fixed

- Explicitly prohibit widget copy and move operations to preserve exclusive child ownership and stable identity, fixing MSVC C2280 errors in Windows shared builds.

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
