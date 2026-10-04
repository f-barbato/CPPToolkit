# platform

OS / platform information utilities.

## API

- `cpptoolkit::platform::OperatingSystem` — enum identifying the current OS.
- `cpptoolkit::platform::GetCurrentOS()` — `constexpr` detection based on compiler macros.
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
