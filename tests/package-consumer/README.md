# Installed-package regression consumer

This is a standalone consumer, not an `add_subdirectory` build of CPPToolkit.
It exercises header-only modules, exported MVVM/net methods and optionally
renders UI widgets using contexts created in the consumer executable.
No graphical window is required.

Use its manifest to exercise the actual overlay installation:

```bash
cmake -S tests/package-consumer -B build/package-dynamic \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux-dynamic \
  -DCPPTOOLKIT_CONSUMER_UI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/package-dynamic
ctest --test-dir build/package-dynamic --output-on-failure
```

On Windows use `x64-windows`; on Linux use `x64-linux` to test static linkage.
The consumer pins raylib 5.5 to match the repository's current validated backend.

Alternatively, after building and installing the library into a staging prefix,
configure without the vcpkg toolchain and pass `CMAKE_PREFIX_PATH` containing
that prefix and the dependency installation prefix. Set
`CPPTOOLKIT_CONSUMER_UI=OFF` for a core-only installation.

For a standalone shared install, the runtime loader also needs the library
directories (CMake discovery alone does not configure transitive runtime lookup):

```bash
LD_LIBRARY_PATH="/path/to/cpptoolkit/lib:/path/to/dependencies/debug/lib:/path/to/dependencies/lib" \
  ctest --test-dir build/installed-shared-consumer --output-on-failure
```

For deployment, co-locate dependencies and configure the application's loader
path/RPATH as appropriate instead of relying on development-machine paths.
