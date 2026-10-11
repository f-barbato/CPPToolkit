# concurrency

General-purpose concurrency primitives, independent of MVVM and UI.

## API

- `CancellationSource` owns shared cancellation state and requests cancellation with `Cancel()`.
- `CancellationToken` is a copyable, read-only handle that checks `IsCancellationRequested()`.
- `Dispatcher` queues actions for the thread calling `ProcessPending()`. `Main()` returns the process-wide dispatcher; each drain processes a FIFO snapshot, deferring newly posted actions to the next drain.
- `NotificationWorker` executes posted tasks in FIFO order on one dedicated background thread. `Instance()` returns the process-wide worker; tasks serialize, and shutdown drains queued work. Uncaught task exceptions terminate the process.

Cancellation is cooperative: a task must check its token and exit safely.
It does not interrupt blocking I/O, forcibly stop a thread or remove queued
work. `Cancel()` is thread-safe and idempotent; a cancelled source cannot be
reset. Create a new source for a new operation.

Tokens remain valid after their source is destroyed. Destroying a source does
not request cancellation. A default-constructed token is never cancelled.
Token copies can be checked concurrently, but assigning to the same token
object while another thread reads it requires external synchronization.

## Usage

Enable the `concurrency` vcpkg feature, or configure standalone builds with
`CPPTOOLKIT_BUILD_CONCURRENCY=ON`. Link `cpptoolkit::concurrency`.

```cpp
#include <cpptoolkit/concurrency/concurrency.h>
#include <thread>

cpptoolkit::concurrency::CancellationSource cancellation;
std::thread worker([token = cancellation.Token()] {
    while (!token.IsCancellationRequested()) {
        DoWorkChunk();
    }
});

// When the operation should stop:
cancellation.Cancel();
worker.join();
```

The same token can be captured by an `ui::Application::RunAsync()` callback;
that API still serializes tasks on its single background worker.

## Dependencies

Standard library and `Threads::Threads`, with no MVVM/UI dependencies. The compiled module follows
`BUILD_SHARED_LIBS` and provides generated `Export.h` annotations for Windows.
Applications creating threads must link their platform's thread library as needed.
