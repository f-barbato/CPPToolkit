# 0.1.0

First stable release.

### Added

- Generic `ui::ShortcutManager` (`Application::Shortcuts()`): unique key chords bound to actions or `mvvm::Command`s, `CanExecute`, rebinding, conflict detection and display text.
- `mvvm::UndoStack` with groups, merged continuous edits, depth limit and bindable state, plus `SetUndoable()`/`RecordChange()`; `SliderFloatWidget`, `DragFloatWidget`, `CheckBoxWidget` and `TextBoxWidget` record user edits through an optional `History` pointer. The demo binds Ctrl+Z / Ctrl+Y and Undo/Redo buttons.
- Independent `concurrency` module, available through the `concurrency` vcpkg feature and `cpptoolkit::concurrency` CMake target, with static/shared builds and Windows export headers.
- Thread-safe cooperative cancellation through `CancellationSource` and copyable `CancellationToken` handles sharing cancellation state.
- TestUI Feedback example demonstrating background task progress, cancellation, restart and safe worker shutdown.
- Regression tests for cancellation, dispatcher snapshot ordering and FIFO background execution.
- Security policy documenting supported versions and vulnerability reporting.
- `Application::SetFont()`, `ResetFont()` and getters to use a custom default font, applied at the next frame boundary.
- `FileDialog` (native open/save/folder dialogs via nativefiledialog-extended, parented to the window on Windows/macOS/X11/Wayland) and `FilePathWidget` in the `ui` module, with non-blocking `*Async` variants (worker thread; synchronous on macOS); the TestUI font control uses it.
- `Panel::SetClosable()`/`IsClosable()` to hide the window (or docked-tab) close button; the TestUI layout controls toggle it.
- `TabBarWidget::SetTabBarFlags()`/`GetTabBarFlags()` to configure reordering, overflow policy and the tab-list button; TestUI's Layout tab exposes them, plus tab shape and a "Style" tab editing global ImGuiStyle metrics (rounding, borders, spacing, sizes, opacity, font scale) with reset.
- `ui::Palette` (primary, secondary, background, surface, text, border) and `Styler::SetPalette()`, which derives every widget color from the palette while preserving style metrics; `Palette::Dark()`/`Light()` presets.
- `CPPTOOLKIT_OS_WINDOWS/MACOS/LINUX/UNKNOWN` preprocessor macros in `cpptoolkit/platform/Platform.h`; `ui` now depends on `platform`.

### Changed

- Reorganize the TestUI demo (`modules/ui/examples`) into `.h`/`.cpp` pairs (`DemoViewModel`, `DemoPanels`, `ControlsPanel`, `SerialConsole`, `TestUI`, `TestUIState`, `CancellationExample`) and group its windows logically: Live demo (Telemetry/Editing tabs), Controls (Appearance/Windows tabs), the widget gallery (tab-bar and style options now in its Style tab) and Serial Console. Features are unchanged.
- Complete Doxygen documentation of `FileDialog`/`PendingFileDialog`; documentation generation is warning-free.

### Changed (breaking changes)

- Move `Dispatcher` and `NotificationWorker` from `cpptoolkit::mvvm` to `cpptoolkit::concurrency`. Replace their `cpptoolkit/mvvm/` includes with `cpptoolkit/concurrency/` includes, or use `cpptoolkit/concurrency/concurrency.h`; they are no longer included by `mvvm.h`.
- Make `mvvm` header-only: no MVVM DLL/shared/static library or generated `mvvm/Export.h` is produced. The `cpptoolkit::mvvm` target remains available.
- Make UI depend on `concurrency` in CMake and vcpkg. Non-UI consumers of the moved queues must explicitly enable and link `concurrency`. Rebuild consumers against the updated package.
- Update release packaging to include the compiled concurrency library instead of the former MVVM library. `Application::Dispatch()` and `RunAsync()` retain their existing execution behavior.

### Fixed

- Configure Linux IntelliSense with GCC and the project compilation database to avoid false override errors caused by obsolete header copies.
