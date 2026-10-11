# 0.1.0-b005

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
