# ui

A retained-mode layer on top of Dear ImGui (immediate-mode), data-bound to `mvvm`.

## API

- `Widget` — base class; `Draw()` is called every frame and is expected to issue the corresponding ImGui calls.
- `Panel` — persistent container of child widgets (`Add<T>(...)`, `Remove(...)`). The widget tree is built once; only the values read in `Draw()` change frame to frame.

`Panel()` remains an inline container, while `Panel("Title")` creates a closable
ImGui window with configurable flags. `DockedPanel` enables docking and hosts
child panels. Override `OnBuild()` to populate new widget trees; `Build()`
invokes the hook and recursively builds children. It remains virtual for
compatibility with existing containers overriding it.

`View<T>` attaches its model as a borrowed ambient context, inherited through
`Add()` and container construction. Descendants can use `GetViewModel<T>()`
and `GetApplication()`; explicitly attached child models are preserved.
Contexts must outlive their widgets and must be set before tree construction.

`Application::Create<Root>()` creates one process-wide application for a root
type; `Instance()` retrieves it, and `GetInstance<Root>()` remains a compatibility
alias. Creating a different root type later throws. Fluent setters support
runtime window configuration. `RunAsync()` queues serialized background work;
`Dispatch()` queues work drained in an active GUI frame on the render thread.
Use owned/weak captures for background work, never dangling widget/model
pointers. Teardown covers exceptions during construction as well as rendering.

### Concrete widgets (`cpptoolkit/ui/widgets/widgets.h`)

- `TextWidget` — read-only text, polls a bound `ObservableProperty<std::string>`.
- `TextBoxWidget` — single-line editable text with two-way binding to `ObservableProperty<std::string>` and an optional `OnTextChanged` callback.
- `SliderFloatWidget` — two-way bound `ImGui::SliderFloat`, writes back via `Set()` on user interaction.
- `ButtonWidget` — bound to a `mvvm::Command`; disabled when `CanExecute()` is false.
- `PlotLineWidget` — real-time line plot (ImPlot) sampling a bound float property every frame into a `cpptoolkit::structs::RingBuffer`.

### Generic widget catalog

All widgets below are included by `widgets.h` and `ui.h`. Related controls
share a category header rather than duplicating one file per small wrapper.

| Header | Widgets | Binding / event |
|---|---|---|
| `InputWidgets.h` | `CheckBoxWidget` | `bool`, `OnCheckedChanged` |
| `InputWidgets.h` | `RadioButtonWidget`, `ComboBoxWidget`, `ListBoxWidget` | `int` selection, `OnSelectionChanged` |
| `InputWidgets.h` | `InputIntWidget`, `InputFloatWidget`, `DragFloatWidget`, `SpinBoxWidget` | numeric property, `OnValueChanged` |
| `InputWidgets.h` | `TextAreaWidget`, `PasswordWidget`, `SearchBoxWidget` | string, `OnTextChanged`, `readOnly` |
| `InputWidgets.h` | `ColorPickerWidget` | `std::array<float,4>` RGBA, `OnColorChanged` |
| `FeedbackWidgets.h` | `ProgressBarWidget`, `SpinnerWidget`, `BadgeWidget` | read-only progress, busy flag, status string |
| `FeedbackWidgets.h` | `NotificationWidget` | message and open flag, `OnDismissed` |
| `FeedbackWidgets.h` | `ImageWidget` | borrowed `ImTextureID` property; caller owns the texture |
| `DataWidgets.h` | `SelectableWidget`, `TableWidget`, `TreeViewWidget` | selection properties, `OnSelectionChanged`; table also `OnSortRequested` |
| `NavigationWidgets.h` | `TabBarWidget`, `BreadcrumbWidget` | selected index, `OnSelectionChanged` |
| `NavigationWidgets.h` | `MenuBarWidget`, `MenuWidget`, `MenuItemWidget`, `ToolbarWidget` | retained menus/toolbars; items bind `Command` and expose `OnClick` |
| `LayoutWidgets.h` | `StackPanel`, `GridPanel`, `ScrollPanel`, `SplitterWidget` | owned children; splitter binds pixel extent and `OnSizeChanged` |
| `OverlayWidgets.h` | `TooltipWidget`, `PopupWidget`, `DialogWidget`, `ContextMenuWidget` | tooltip follows previous item; popups bind open flag, `OnOpened` / `OnClosed` |

`ButtonWidget` now also exposes `OnClick`, after command execution, and
`SliderFloatWidget` exposes `OnValueChanged`, after property commit.
Existing constructor calls remain valid.

### Binding rules

Input widgets borrow `mvvm::ObservableProperty<T>` and poll it every frame.
An actual user edit calls `Set()` **before** the optional callback, synchronously
on the render thread. Programmatic `Set()` calls do not trigger widget
user-edit callbacks. Use the property's RAII `OnChanged()` subscription for
changes from any source, and keep that subscription alive.

Properties/commands must outlive their widgets. Widget trees, event assignments
and rendering are render-thread-only. Do not remove/add children inside a
callback while the tree is being drawn; defer structural mutations.

Combo/list choices and tree nodes are immutable copies supplied at construction.
Combo/list selection `-1` means none; invalid initial indices throw, and invalid
programmatic indices are visibly diagnosed rather than indexed. Tree IDs must
be unique nonnegative integers. Invalid or `-1` table/tree selection displays no
selection. A table's missing cells display blank; extra cells are not drawn.

`TableWidget::OnSortRequested` passes ordered `{Column, Ascending}` keys to the
ViewModel, including the initial default sort. The ViewModel applies sorting
and decides whether to clear or remap row selection. No hidden data mutation
occurs in the widget.

Use `TabBarWidget::AddTab()` to populate tabs and
`SplitterWidget::First()` / `Second()` to populate panes. The inherited direct
`Add()` is not a content insertion point for these two specialized containers.
A tab selection must be a valid zero-based tab index. A window-local
`MenuBarWidget` requires `ImGuiWindowFlags_MenuBar` on its host window.
Menu shortcut strings are visual hints, not automatic keyboard bindings.

Put `TooltipWidget` and `ContextMenuWidget` immediately after their target item.
Keep popups in a stable, continuously rendered window/ID scope if commands
elsewhere can open them. Modal titles must be unique. External visibility
changes do not emit popup transition callbacks; user dismissals do.

Numeric/text inputs expose `readOnly`; `SpinBoxWidget` clamps user edits to its
configured limits without rewriting programmatic values. Text input buffers
grow automatically. Password masking hides characters on screen only: the
property still stores ordinary plaintext, not a protected credential container.
Progress values are visually clamped to [0,1], and invalid nonfinite progress
or splitter values display a diagnostic. Layout dimensions and ranges are
documented and validated where required.

## Concept

ImGui itself stays immediate-mode under the hood (it must be called every frame), but the **structure** of the UI becomes retained: you build the `Panel`/`Widget` tree once, bind widgets to `cpptoolkit::mvvm::ObservableProperty<T>` instances, and just call `Draw()` in the render loop. Widgets read bound properties every frame (`Get()`), which is thread-safe by construction — see the `mvvm` module's thread-safety notes.

## Optional sub-functionality

Only the explicit `ui-net-widgets` feature / `CPPTOOLKIT_BUILD_UI_NET_WIDGETS`
option defines `CPPTOOLKIT_UI_HAS_NET_WIDGETS` and adds the net dependency.
Enabling `ui` and `net` separately does not activate this combined feature.

## Dependencies

- `cpptoolkit::mvvm`
- `imgui` (vcpkg feature `docking-experimental`)
- `implot`

The current `Application` API exposes raylib and rlImGui, so the UI module also
requires raylib (at least 5.5), GLFW and the pinned rlImGui bridge. These are
installed/exported with the package as required; consumers need not fetch a
second bridge. The bridge's installed headers include their font icon and
generated export headers.

For shared UI builds use the complete `ports/` overlay and a dynamic triplet.
ImGui and ImPlot must be shared too, preserving one set of context globals
across the executable, bridge and widget library. The library rejects
shared-UI configurations with static GUI dependencies.

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

## Example

### Text box

```cpp
mvvm::ObservableProperty<std::string> name{nullptr, "Name", ""};
ui::Panel panel;
auto& textBox = panel.Add<ui::TextBoxWidget>("Name", name);
textBox.OnTextChanged = [](const std::string& text) {
    // Handle user edits, e.g. validate or filter using the new text.
};
```

The callback can also be passed as the third constructor / `Add` argument.
`OnTextChanged` runs synchronously on the render thread, after the bound
property has been updated, only when the user changes the text (including
clearing it). Programmatic `Set()` calls are reflected by polling during
`Draw()` and do not trigger this callback; use the property's `OnChanged()`
subscription to observe changes from any source. The property must outlive
the widget. The UTF-8 input buffer grows automatically, without a fixed
character limit.

### Application

`examples/main.cpp` and `examples/TestUI.h` (built with
`CPPTOOLKIT_BUILD_EXAMPLES=ON`) provide TestUI, the `cpptoolkit_ui_demo`
executable. In addition to the original sensor plot, slider and text boxes,
it contains Input, Feedback, Data, Navigation, Layout and Overlay tabs covering
every generic widget. Callback messages are themselves ViewModel-bound text.
The table demonstrates ViewModel sorting and the image displays the ImGui font
atlas without allocating another texture.

The backend uses raylib, Dear ImGui via
[rlImGui](https://github.com/raylib-extras/rlImGui), and ImPlot.
When net is enabled, a separate serial-console panel demonstrates asynchronous
I/O handoff and dispatched error visibility; the actual serial transport is
still a placeholder and connection attempts report that explicitly.
Headless gtests exercise user edits, callback ordering, programmatic updates,
read-only behavior, UTF-8 buffers, popups and the complete TestUI gallery:

```bash
cmake --build build/all-modules-debug --target cpptoolkit_ui_tests cpptoolkit_ui_demo
ctest --test-dir build/all-modules-debug --output-on-failure
```

See the root README for generating Doxygen documentation for every module.
