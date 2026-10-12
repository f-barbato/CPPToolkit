# platform

OS / platform information utilities.

## API

- `CPPTOOLKIT_OS_WINDOWS`, `CPPTOOLKIT_OS_MACOS`, `CPPTOOLKIT_OS_LINUX`, `CPPTOOLKIT_OS_UNKNOWN` — preprocessor macros, each `1` or `0` (exactly one is `1`), for use in `#if`.
- `cpptoolkit::platform::OperatingSystem` — enum identifying the current OS.
- `cpptoolkit::platform::GetCurrentOS()` — `constexpr` detection based on the macros above.
- `cpptoolkit::platform::ToString(OperatingSystem)` — human-readable name.

## Usage

```cpp
#include <cpptoolkit/platform/Platform.h>

if (cpptoolkit::platform::GetCurrentOS() == cpptoolkit::platform::OperatingSystem::Linux) {
    // ...
}
```

## Dependencies

None (header-only, no external dependencies).
