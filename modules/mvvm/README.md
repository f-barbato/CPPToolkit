# mvvm

MVVM pattern for C++, adapted to immediate-mode UI rendering (see the `ui` module).

## API

- `Observable<T>` — multi-subscriber observable with RAII `Subscription` (auto-unsubscribe on destruction).
- `ObservableObject` — ViewModel base class; raises a generic "property changed by name" notification, mirroring `CommunityToolkit.Mvvm`'s `ObservableObject`.
- `ObservableProperty<T>` — mutex-protected observable property, analogous to `[ObservableProperty]`. Notifies both its own subscribers and its owning `ObservableObject`.
- `Command` — action + optional `CanExecute` guard, analogous to `ICommand`.
- `UndoStack` — undo/redo history (not thread-safe; use from the render thread). `Execute()` applies and records an action, `Record()` records one already applied; groups (`BeginGroup`/`UndoStack::Group`) form a single step, `BreakMerge()` ends a continuous edit, the depth is limited, and bindable `CanUndo`/`CanRedo`/`UndoText`/`RedoText` plus `UndoCommand`/`RedoCommand` drive menus. `SetUndoable()`/`RecordChange()` record `ObservableProperty` changes, merging consecutive ones (slider drags). Changes made by subscribers while undoing are not recorded. Widgets expose a `History` pointer to record user edits.

## Thread-safety model

- `ObservableProperty<T>::Get()`/`Set()` are mutex-protected. UI widgets are expected to **poll** `Get()` every frame inside their `Draw()` call — this is sufficient and safe without any marshaling, since immediate-mode UIs redraw every frame regardless.
- `concurrency::Dispatcher` is the exception path: use it only if a handler needs to call a UI API directly (e.g. `ImGui::OpenPopup`) from a thread other than the render thread. It belongs to the independent `concurrency` module, as does `concurrency::NotificationWorker`.

## Dependencies

- `cpptoolkit::struct` (ring buffer, used internally).

This module is header-only and does not depend on `concurrency`.

## Usage

```cpp
#include <cpptoolkit/mvvm/mvvm.h>

class RegisterViewModel : public cpptoolkit::mvvm::ObservableObject {
public:
    cpptoolkit::mvvm::ObservableProperty<int> Address{this, "Address", 0};
    cpptoolkit::mvvm::ObservableProperty<std::uint32_t> Value{this, "Value", 0};
};
```
