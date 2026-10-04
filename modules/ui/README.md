# ui

A retained-mode layer on top of Dear ImGui (immediate-mode), data-bound to `mvvm`.

## API

- `Widget` — base class; `Draw()` is called every frame and is expected to issue the corresponding ImGui calls.
- `Panel` — persistent container of child widgets (`Add<T>(...)`, `Remove(...)`). The widget tree is built once; only the values read in `Draw()` change frame to frame.

## Concept

ImGui itself stays immediate-mode under the hood (it must be called every frame), but the **structure** of the UI becomes retained: you build the `Panel`/`Widget` tree once, bind widgets to `cpptoolkit::mvvm::ObservableProperty<T>` instances, and just call `Draw()` in the render loop. Widgets read bound properties every frame (`Get()`), which is thread-safe by construction — see the `mvvm` module's thread-safety notes.

## Optional sub-functionality

When both `ui` and `net` are enabled (vcpkg feature `ui-net-widgets`), `CPPTOOLKIT_UI_HAS_NET_WIDGETS` is defined and widgets that visualize `net` transport state become available.

## Dependencies

- `cpptoolkit::mvvm`
- `imgui` (vcpkg feature `docking-experimental`)
- `implot`

Graphics backend (window/input/rendering) is left to the consumer; the reference combination used during design is **raylib 6.0 + rlImGui** (rlImGui is not on vcpkg — vendor it via `FetchContent` in your application, not inside this library).

## Usage

```cpp
#include <cpptoolkit/ui/ui.h>

class RegisterView : public cpptoolkit::ui::Panel {
public:
    explicit RegisterView(RegisterViewModel& vm) {
        // Add<MyWidget>(...) widgets bound to vm's ObservableProperty members
    }
};
```
