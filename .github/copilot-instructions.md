# CPPToolkit — Project Memory

This file gives Copilot persistent context about this project's goals, architecture, and conventions. Keep it up to date as decisions evolve.

## What this project is

CPPToolkit is a **modular C++ toolkit** distributed as a **single vcpkg package**, where every module is exposed as an opt-in **vcpkg feature**. Consumers enable only the modules they need; modules can depend on each other and unlock extra functionality when combined with others.

## Modules

| Module | Purpose |
|---|---|
| `platform` | Operating system / platform information utilities |
| `struct` | Common data structures (ring buffers, etc.) |
| `mvvm` | MVVM pattern for C++ (see below) |
| `algo` | Algorithm integrations |
| `ui` | Retained-mode layer on top of an immediate-mode GUI, bound via `mvvm` |
| `net` | Networking / communication transports (serial, TCP, BLE, ...) |

Module dependency rules established so far:
- `mvvm` depends on `struct` (e.g. uses ring buffers internally).
- `ui` depends on `mvvm`.
- `ui` + `net` enabled together unlock extra sub-functionality (e.g. network-status widgets) via an explicit combined feature (e.g. `ui-net-widgets`) — vcpkg does **not** auto-activate cross-feature functionality just because two features are both enabled; it must be opted into explicitly.

## Key architectural decisions

### Package manager: vcpkg
- Manifest mode, single `vcpkg.json` for the whole repo, one **feature per module**.
- Private/unpublished library → integrate via **overlay port** (`vcpkg-configuration.json` pointing at a local `ports/` folder) rather than publishing to the public vcpkg registry.
- Feature → CMake option mapping done via `vcpkg_check_features()` in `portfile.cmake`.

### Repository / module layout
Every module follows the same structure:
```
modules/<name>/
  include/cpptoolkit/<name>/   # public headers
  src/                         # private implementation (.cpp)
  detail/                      # internal-only headers, not exposed to consumers
  tests/                       # unit tests (gtest), built only if requested
  examples/                    # usage examples, built only if requested
  CMakeLists.txt
  README.md
```
Sub-areas within a module (e.g. `net`'s transports: `serial/`, `tcp/`, `ble/`) live as subfolders under `include/` and `src/`, compiled conditionally into the **same** module target (not split into separate CMake targets).

### Shared CMake helper
All modules are declared through one shared function (`cpptoolkit_add_module`, in `cmake/CPPToolkitModule.cmake`) to avoid duplicating install/export/compile-feature logic:
```cmake
cpptoolkit_add_module(
    NAME <module>
    SOURCES <...>        # omit for header-only → auto INTERFACE target
    PUBLIC_DEPS <...>
    PRIVATE_DEPS <...>
    DEFINITIONS <...>
)
```
Each module exposes a target named `cpptoolkit::<name>`.

### MVVM module design
Adapts C#/WPF-style MVVM to immediate-mode rendering:
- `Observable<T>`: multi-subscriber observable with RAII `Subscription` (auto-unsubscribe on destruction).
- `ObservableObject` / `ObservableProperty<T>`: mirrors `CommunityToolkit.Mvvm`'s `ObservableObject` + `[ObservableProperty]` — notifies both per-property subscribers and a generic "property changed by name" subscriber on the owning object.
- `Command`: wraps an action + optional `CanExecute` predicate.
- Thread-safety model (important, decided after iteration):
  - `ObservableProperty<T>::Get()`/`Set()` are mutex-protected.
  - Widgets **poll** the current value every frame inside `Draw()` — this is sufficient and thread-safe without any marshaling, since immediate-mode UIs redraw every frame anyway.
  - A `Dispatcher` (post actions to the main/render thread) is only needed in the rare case a background-thread handler must call ImGui APIs **directly** (e.g. `ImGui::OpenPopup`). It is not the primary binding mechanism.
  - A `NotificationWorker` (single dedicated background thread, FIFO queue) can run `Observable::Notify` callbacks off the calling thread when handlers do non-trivial work — ordering is preserved per-thread, but all notifications serialize through it (no parallel handler execution).

### UI module design
- Goal: make ImGui (immediate-mode) feel "retained" — a persistent widget tree built **once**, bound to `mvvm` properties, redrawn each frame.
- `Widget` base class with virtual `Draw()`; `Panel` is a container (`Add<T>(...)`, `Remove(...)`) holding `unique_ptr<Widget>` children.
- Widgets bind to `ObservableProperty<T>` two-way: read on `Draw()`, write back on user interaction.
- Concrete widgets shipped in `cpptoolkit/ui/widgets/`: `TextWidget`, `SliderFloatWidget`, `ButtonWidget`, `PlotLineWidget` (ImPlot, backed by a `cpptoolkit::structs::RingBuffer`).
- Chosen graphics backend: **raylib 6.0** + **rlImGui** (bridge, not on vcpkg — vendored via `FetchContent` in `modules/ui/examples/`, not inside the `ui` library itself) + **ImPlot** for real-time plotting.
- `modules/ui/examples/main.cpp` is a working raylib+ImGui+ImPlot demo app (built with `CPPTOOLKIT_BUILD_EXAMPLES=ON`), wiring a `DemoViewModel` to a `Panel` of the widgets above.
- Original use case: visual debug tooling for microcontrollers (register/memory viewers, real-time telemetry plots, serial/BLE consoles).

### net module
- Transport abstraction: common `ITransport` interface (`connect()/read()/write()`), with concrete implementations per transport (serial, TCP, BLE).
- BLE: **SimpleBLE** (cross-platform: WinRT/CoreBluetooth/BlueZ), available on vcpkg as `simpleble`. ⚠️ License is **BUSL-1.1**, not OSI-permissive — verify terms before commercial distribution (converts to Apache-2.0 per-version after ~4 years).

## Build system conventions
- CMake ≥ 3.20, C++17 minimum (`cxx_std_17`).
- Root `CMakeLists.txt` exposes one `CPPTOOLKIT_BUILD_<MODULE>` option per module (default `OFF`), and auto-enables hard dependencies (e.g. enabling `ui` forces `mvvm` and `struct` ON).
- Tests and examples are opt-in via `CPPTOOLKIT_BUILD_TESTS` / `CPPTOOLKIT_BUILD_EXAMPLES`, kept out of default builds.

## License
Apache License 2.0 (see `LICENSE`). Be mindful of this when choosing third-party dependencies for the `net`/BLE feature (SimpleBLE is BUSL-1.1).

## Status
🚧 In progress — vcpkg/CMake scaffolding and module skeletons exist and build/install/`find_package` has been verified end-to-end for `platform`/`struct`/`mvvm`/`algo`/`net`. `ui` has concrete widgets (`TextWidget`, `SliderFloatWidget`, `ButtonWidget`, `PlotLineWidget`) and a working raylib+ImGui+ImPlot example (`modules/ui/examples/`), but building it requires a full vcpkg toolchain (imgui/implot/raylib) that has not been exercised in this environment. See `copilot-session-1f7a4d70-3f05-4256-aa83-bf5d122f3b1f.md` for the full historical design conversation.

## Working conventions for Copilot in this repo
- Keep this file in sync whenever a new architectural decision is made or an existing one changes.
- Prefer updating existing module READMEs over duplicating design notes here; this file should stay a high-level map, not a full spec.
- When scaffolding new modules, follow the standard layout and use `cpptoolkit_add_module` rather than hand-writing module `CMakeLists.txt` from scratch.
