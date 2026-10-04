# struct

Common data structures used across other CPPToolkit modules.

## API

- `cpptoolkit::structs::RingBuffer<T, Capacity>` — fixed-capacity circular buffer, overwrites the oldest element once full. Used internally by `mvvm` and useful for buffering telemetry samples.

## Usage

```cpp
#include <cpptoolkit/struct/RingBuffer.h>

cpptoolkit::structs::RingBuffer<float, 256> samples;
samples.Push(3.14f);
auto value = samples.Pop();
```

## Dependencies

None (header-only, no external dependencies).
