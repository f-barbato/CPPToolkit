# mvvm

MVVM pattern for C++, adapted to immediate-mode UI rendering (see the `ui` module).

## API

- `Observable<T>` — multi-subscriber observable with RAII `Subscription` (auto-unsubscribe on destruction).
- `ObservableObject` — ViewModel base class; raises a generic "property changed by name" notification, mirroring `CommunityToolkit.Mvvm`'s `ObservableObject`.
- `ObservableProperty<T>` — mutex-protected observable property, analogous to `[ObservableProperty]`. Notifies both its own subscribers and its owning `ObservableObject`.
- `Command` — action + optional `CanExecute` guard, analogous to `ICommand`.
- `Dispatcher` — marshals actions onto whichever thread calls `ProcessPending()` (typically the render thread). Only needed when a background-thread handler must call UI APIs directly.
- `NotificationWorker` — single dedicated background thread that runs posted tasks in FIFO order; can be used to run `Observable<T>::Notify` handlers off the calling thread.

## Thread-safety model

- `ObservableProperty<T>::Get()`/`Set()` are mutex-protected. UI widgets are expected to **poll** `Get()` every frame inside their `Draw()` call — this is sufficient and safe without any marshaling, since immediate-mode UIs redraw every frame regardless.
- `Dispatcher` is the exception path: use it only if a handler needs to call a UI API directly (e.g. `ImGui::OpenPopup`) from a thread other than the render thread.

## Dependencies

- `cpptoolkit::struct` (ring buffer, used internally).
- `Threads::Threads` (for `NotificationWorker`).

## Usage

```cpp
#include <cpptoolkit/mvvm/mvvm.h>

class RegisterViewModel : public cpptoolkit::mvvm::ObservableObject {
public:
    cpptoolkit::mvvm::ObservableProperty<int> Address{this, "Address", 0};
    cpptoolkit::mvvm::ObservableProperty<std::uint32_t> Value{this, "Value", 0};
};
```
