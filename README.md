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
cmake -B build -DCPPTOOLKIT_BUILD_MVVM=ON -DCPPTOOLKIT_BUILD_UI=ON
cmake --build build
```

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

## Status

🚧 Work in progress — module skeletons and build configuration are being actively developed.

## License

Licensed under the [Apache License 2.0](LICENSE).