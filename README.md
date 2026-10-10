# CPPToolkit

A modular C++ toolkit distributed as a single [vcpkg](https://vcpkg.io) package, with each module exposed as an opt-in feature. Pick only what you need — modules can depend on each other and unlock extra sub-functionality when combined.

## Modules

| Module | Description |
|---|---|
| `platform` | Operating system and platform information utilities |
| `struct` | Common data structures (ring buffers, etc.) |
| `mvvm` | MVVM pattern for C++: `Observable<T>`, `ObservableObject` / `ObservableProperty<T>`, `Command`, thread-safe dispatching |
| `algo` | Collection of algorithm integrations |
| `ui` | A "retained-mode" layer on top of immediate-mode GUIs (Dear ImGui), with data binding to `mvvm` |
| `net` | Networking and communication transports (serial, TCP, BLE, ...) |

## Design goals

- **Single repository, single vcpkg port** — all modules live together and are versioned as one library, while remaining independently selectable via [vcpkg features](https://learn.microsoft.com/en-us/vcpkg/concepts/package-name-versioning#features).
- **Explicit dependencies between modules** — e.g. `ui` depends on `mvvm`, `mvvm` depends on `struct`. Combined features (e.g. `ui` + `net`) can unlock additional functionality (such as network-status widgets) when both are enabled.
- **Consistent module layout** — every module follows the same folder structure (`include/`, `src/`, `detail/`, `tests/`, `examples/`) and is declared through a shared CMake helper function to avoid duplication.
- **Opt-in build footprint** — tests and examples are only built when explicitly requested, keeping default builds minimal and fast.

## Getting started

### Using vcpkg

This private package uses overlay ports. In the consumer project, add
`vcpkg-configuration.json` (adjust paths to the CPPToolkit checkout):

```json
{
  "overlay-ports": ["../CPPToolkit/ports"],
  "overlay-triplets": ["../CPPToolkit/triplets"]
}
```

Keep the complete overlay directory: its ImGui/ImPlot ports support both
static and shared libraries. See [overlay maintenance notes](ports/README.md).

Add `cpptoolkit` to your `vcpkg.json`, enabling only the features you need:

```json
{
  "dependencies": [
    { "name": "cpptoolkit", "features": ["mvvm", "ui"] }
  ]
}
```

Then, in your `CMakeLists.txt`:

```cmake
find_package(cpptoolkit CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE cpptoolkit::mvvm cpptoolkit::ui)
```

### Building from source

```bash
git clone https://github.com/f-barbato/CPPToolkit.git
cd CPPToolkit
cmake --preset all-modules-debug
cmake --build --preset all-modules-debug
```

Set `VCPKG_ROOT` to the vcpkg checkout before configuring presets.

### Static and shared libraries

Compiled modules (`mvvm`, `net`, `ui`) follow `BUILD_SHARED_LIBS` (default
`OFF`). `platform`, `struct` and `algo` are header-only and remain interface
targets. Each compiled module produces its own library, not one monolithic DLL.
Generated `Export.h` headers provide Windows exports/imports and static-build
definitions automatically through the CMake targets; do not set these macros
manually. The exported build interface requires C++23.

The overlay port maps the target triplet's linkage to `BUILD_SHARED_LIBS`.
For a consumer:

```bash
# Linux shared modules and shared GUI dependencies:
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux-dynamic
```

On Windows use `x64-windows` for DLLs or `x64-windows-static` for static libraries.
Linux's standard `x64-linux` triplet is static. For standalone builds of this
repository, use `all-modules-shared-linux` or `all-modules-shared-windows` presets.
Release optimization and linkage are independent: `all-modules-release` no
longer implicitly requests shared linkage.

Shared UI builds require shared ImGui and ImPlot, so the executable, bridge and
widget DLL use the same GUI contexts. Stock vcpkg ImGui/ImPlot ports are
static-only; the included overlays remove this limitation while retaining their
features. An incompatible shared-UI configuration fails with a clear diagnostic.
The pinned rlImGui bridge follows the module linkage and uses independent
exports, with PIC enabled for compiled targets.

Install the configured build with `cmake --install build-directory --prefix
install-prefix`. Consumers use `find_package(cpptoolkit CONFIG REQUIRED)` and
the exported module targets; dependency packages must also be discoverable.
Use a consistent compiler, architecture, C++ ABI and runtime across the library
and application. For deployment distribute all required DLLs/shared libraries,
not only CPPToolkit's; on Windows put DLLs alongside the executable or on PATH,
and on Linux configure the loader path/RPATH appropriately.
Installed libraries default to `$ORIGIN` (`@loader_path` on macOS) for
co-located dependencies, unless a custom `CMAKE_INSTALL_RPATH` is supplied.

The current overlay compiles from its enclosing CPPToolkit checkout. It is not
yet a remote, tagged registry package. Root-manifest version overrides are not
inherited by other projects. Serial/TCP and BLE characteristic I/O remain
placeholders independent of the chosen linkage.

An independent regression consumer is available in
[`tests/package-consumer`](tests/package-consumer/README.md).

## API documentation

All public module headers contain Doxygen documentation, including ownership,
thread-safety and callback contracts. Generate HTML and XML with Doxygen 1.9.5+
(no UI dependencies are needed for this documentation-only configuration):

```bash
cmake -S . -B build/docs -DCPPTOOLKIT_BUILD_PLATFORM=ON -DCPPTOOLKIT_BUILD_DOCS=ON
cmake --build build/docs --target cpptoolkit_docs
```

Open `build/docs/docs/html/index.html`. All module APIs are included regardless
of the enabled build modules; documentation warnings fail the target.
`CPPTOOLKIT_BUILD_DOCS` defaults to `OFF` and does not affect consumer builds.

The UI example (`cpptoolkit_ui_demo`, also called TestUI) showcases the generic
widgets in six tabs. See [the UI module](modules/ui/README.md) for the catalog
and MVVM event binding rules.

## Binary releases

The `Shared library release` GitHub Actions workflow runs when a tag matching
`release/*` is **pushed to GitHub**. It validates the exact supported forms:
`release/1.2.3`, `release/1.2.3-a001`, `release/1.2.3-b001`.
Alpha/beta suffixes accept one or more digits and publish a GitHub prerelease;
stable versions publish a normal release. Unsupported tags fail validation.

Native build jobs produce shared Release packages for Linux, Windows and macOS
on **x64 and ARM64**. The repository must have access to the listed GitHub-hosted
ARM64/macOS runners (private repositories may require an eligible plan).
The six ZIPs and SHA-256 checksums are published only after all build/consumer
checks succeed. No release is created for a partially successful matrix.

Each ZIP preserves `dist/Release/<OS>/<architecture>/`, with OS names `Linux`,
`Windows`, `Darwin` and normalized architecture names `x64`, `arm64`:

```text
dist/Release/Linux/x64/
  include/       # public and generated export headers, dependency headers
  lib/           # shared libraries and CMake package metadata
  bin/           # dependency executables / Windows DLLs, when present
  share/         # dependency package configs and licenses
  LICENSE
  CHANGELOG.md
  HISTORY.md
```

Packages include all modules and the explicit UI/net combination; optional BLE
is excluded (including its non-permissive SimpleBLE dependency). Tests, examples
and debug libraries are not shipped. Public vcpkg dependencies and their license
notices are bundled. OS graphics/system libraries and the platform's C++ runtime
remain prerequisites; these are native binaries, not universal ABI-compatible
packages. Linux packages are built on Ubuntu 24.04.

Use the extracted platform directory as `CMAKE_PREFIX_PATH` to resolve both
CPPToolkit and its dependencies without a vcpkg toolchain. Add its `bin` to
Windows `PATH`, or its `lib` to the platform library search path when needed.

Before creating a release tag:

1. Update the CMake project version and both root/overlay vcpkg package versions.
2. Replace `CHANGELOG.md` with **only that version's notes**, starting with
   `# 1.2.3` (or the complete alpha/beta version). Use `### Added`, `### Fixed`,
   etc. for categories.
3. Prepend the same notes to `HISTORY.md` under `## 1.2.3`, retaining older entries.
4. Commit those files, then create and push the tag:

```bash
python scripts/release.py validate --tag release/0.1.0-b002
git tag release/0.1.0-b002
git push origin release/0.1.0-b002
```

GitHub Release notes come **only from `CHANGELOG.md`**. The workflow checks that
the corresponding entry in `HISTORY.md` agrees, but never includes historical
entries in the release body and never commits changes back to the repository.
An existing GitHub Release is not overwritten; remove a failed/obsolete release
explicitly before rerunning publication.

Release helper tests: `python -m unittest discover -s scripts -p 'test_release.py'`.

## Status

🚧 Work in progress — module skeletons and build configuration are being actively developed.

## License

Licensed under the [Apache License 2.0](LICENSE).