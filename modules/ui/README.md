# ui

A retained-mode layer on top of Dear ImGui (immediate-mode), data-bound to `mvvm`.

## API

- `Widget` — base class; `Draw()` is called every frame and is expected to issue the corresponding ImGui calls.
- `Panel` — persistent container of child widgets (`Add<T>(...)`, `Remove(...)`). The widget tree is built once; only the values read in `Draw()` change frame to frame.

Widgets are explicitly noncopyable and nonmovable: child ownership is exclusive,
and ambient contexts, callbacks and ImGui IDs rely on stable widget identity.
Transfer ownership through smart pointers rather than copying widget objects.

### Editor workspace

`EditorLayoutWidget` creates a dockable viewport workspace with `Left()`,
`Right()`, `Top()`, `Bottom()` and `Center()` panel accessors. Select an
`EditorLayout` preset at construction or with `SetLayout()`. Populate them with the
usual widgets, all sharing the ambient ViewModel and rendering lifecycle:

```cpp
auto& editor = Add<ui::EditorLayoutWidget>("Main editor", ui::EditorLayout::LeftFullHeight);
editor.Left().Add<ui::TextWidget>("Project");
editor.Right().Add<ui::SliderFloatWidget>("Speed", vm.Speed, 0.1f, 4.0f);
editor.Top().Add<ui::ButtonWidget>("Pause", vm.Pause);
editor.Bottom().Add<ui::TextWidget>(vm.Status);
editor.Center().Add<ui::RenderTextureWidget>("Screen", 256, 240, renderScreen);
editor.SetPanelVisible(ui::EditorRegion::Right, false);
```

| `EditorLayout` preset | Initial arrangement |
|---|---|
| `TopBottomFullWidth` (default) | Top/bottom span the width; sidebars flank the center |
| `LeftFullHeight` | Left spans the height; top/bottom start beside it |
| `RightFullHeight` | Right spans the height; top/bottom end beside it |
| `SidebarsFullHeight` | Both sidebars span the height; top/bottom lie between them |

`SetLayout()` rebuilds docking on the next visible frame only when the preset
changes, preserving panel visibility. `GetLayout()` reports the selected preset,
not later user-adjusted positions. `SetPanelVisible(region, false)` hides/closes
an area; `true` shows/reopens it. `IsPanelVisible()` also reflects closing a panel
with its window button, independently of the workspace's own `Visible` flag.
These methods do not change `RenderEnabled`. Empty docking areas are reclaimed
by ImGui when their windows are closed; reopening retains their docking IDs.

TestUI includes a separate **Editor layout controls** window with buttons for
all four presets, visibility toggles for each of the five panels and a layout
reset. This window is independent of the five areas, so hiding the top panel
does not remove access to the controls.

The initial layout is created only when no docking node exists. ImGui ini
settings preserve later resizing/docking; `ResetLayout()` restores the defaults
and reopens the five panels on the next visible frame. Use a stable unique
workspace identifier and one visible viewport workspace at a time.
Saved docking settings take precedence over the constructor preset; use
`ResetLayout()` to apply the configured preset to an existing saved workspace.
Keep the `###` ID suffix when changing a panel's display title via `SetTitle()`.
Closing/hiding panels affects `Draw()`, not the pre-ImGui `Render()` phase;
`RenderEnabled` controls the latter. Inherited `Add()` can host extra windows.

### Raylib rendering stage

The widget lifecycle is `PreBuild()` / `Build()`, then `Render()` and `Draw()`
each frame, followed by `Destroy()` before graphics shutdown. `Application`
clears the screen, calls the root's `Render()` inside the raylib drawing scope,
then starts ImGui and calls `Draw()`. Backend frame scopes close even if a
widget throws. `Dispatcher` remains inside the ImGui frame: dispatched changes
are seen by the following frame's `Render()`.
Standard containers complete all child teardown hooks and release ownership
even if one hook fails, then rethrow the first exception for error reporting.

Override `OnRender()` for direct raylib drawing or texture uploads. The default
`Render()` calls that hook and recursively visits children, including tabs and
splitter panes. This phase is independent of `Visible`, collapsed windows,
closed popups and selected tabs. Set `RenderEnabled = false` to suspend a
subtree explicitly. Rendering is not simulation: an emulator's clock/state
must not depend on GUI visibility or one update per displayed frame.

`RenderTextureWidget` owns a fixed-resolution `RenderTexture2D`. It creates it
in `Build()`, clears and renders into it in `Render()`, displays it in `Draw()`
with vertically corrected UVs, and releases it in `Destroy()` or destruction.
The display size can change without reallocating the framebuffer. Its callback
can access the same ambient ViewModel as sibling MVVM controls:

```cpp
// Inside OnBuild() of a View or container with an EmulatorViewModel context:
auto& screen = Add<ui::RenderTextureWidget>("Console", 256, 240,
    [](ui::RenderTextureWidget& widget) {
        auto* vm = widget.GetViewModel<EmulatorViewModel>();
        if (!vm) throw std::logic_error("Missing EmulatorViewModel");
        // Issue raylib drawing calls based on vm's current state here.
    });
screen.SetDisplaySize(ImVec2(512, 480));
```

The callback runs with the offscreen target already active: do not call
`BeginDrawing`/`EndDrawing`, change render targets, or call ImGui. Balance any
camera/shader scopes you open. Child widgets render after the target is restored;
they are not automatically drawn into this framebuffer. Texture creation,
updates and release are render-thread-only, and destruction must happen before
the raylib window closes. Resource-owning widgets are noncopyable.

For a CPU-generated emulator framebuffer, a custom widget can upload a stable
pixel snapshot in `OnRender()` and display the texture in `Draw()`. Keep GPU
resources in the widget; keep emulation state in the model/ViewModel. Use proper
snapshot synchronization or double buffering for background-produced pixels.
The demo's separate "Raylib framebuffer" panel shares `DemoViewModel` with
TestUI, so its amplitude slider controls both the plot and offscreen scene.

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
type; `Instance()` retrieves it, and `GetInstance<Root>()` remains a
compatibility alias. Creating a different root type later throws. Fluent
setters support runtime window configuration. `RunAsync()` queues serialized
background work on one worker thread; it does not run each task in parallel.
Use a dedicated thread or thread pool for independent parallel tasks.
The UI module enables and links `concurrency` for its work queues.
Pass a `concurrency::CancellationToken` to long-running
work and check `IsCancellationRequested()` regularly to stop cooperatively;
cancellation does not interrupt work or remove a task from the queue.
`Dispatch()` queues work drained in an active GUI frame on the render thread.
Use owned/weak captures for background work, never dangling widget/model
pointers. Teardown covers exceptions during construction as well as rendering.

TestUI's **Feedback** tab also includes
a cancellation example (`examples/CancellationExample.h/.cpp`). **Start task**
launches a dedicated background thread and updates bound progress/status
properties. **Cancel task** requests cancellation through a `CancellationSource`;
the worker captures its token and checks it after each bounded work chunk.
Start is disabled during execution, and a fresh source is created for each run.
The widget cancels and joins its worker during teardown/destruction, keeping
the captured state alive until the worker exits. The UI module depends on `concurrency`, so the example is included with UI builds.

### Global styling

`Styler` owns an `ImGuiStyle` and supports `Theme::Dark`, `Theme::Light` and
`Theme::Classic`. Construction and customization do not require an ImGui
context. `GetStyle()` exposes all ImGui colors and metrics, including padding,
spacing, borders and rounding. `SetTheme()` replaces the color palette but
preserves customized metrics.

```cpp
ui::Styler styler(ui::Theme::Dark);
styler.GetStyle().WindowRounding = 8;
styler.GetStyle().FrameBorderSize = 1;
styler.GetStyle().ItemSpacing = ImVec2(10, 8);
styler.GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.4f, 0.7f, 1);
ui::Application::Create<Root>().SetStyler(styler);
```

`Application::SetStyler()` copies the configuration and applies it after backend
initialization, before building widgets. Runtime changes are applied at the
start of the next frame, outside any widget style stacks. To edit the current
configuration, copy `GetStyler()`, customize it, then call `SetStyler()` again.
All style operations are render-thread-only; background code must dispatch
them with `Application::Dispatch()`. `Styler::Apply()` is available for standalone
ImGui usage and requires a current context and no active widget/style scopes.
The legacy `SetDarkTheme()` selects Dark/Light while preserving metrics; the
application still defaults to Light. TestUI's editor layout controls include
buttons for switching all three themes, three palette presets and one color picker per palette color.

### Semantic palette

`ui::Palette` defines `Primary`, `Secondary`, `Background`, `Surface`, `Text`,
`TextDisabled` and `Border`. `Styler::SetPalette()` derives every ImGui color
(buttons, inputs, tabs, headers, sliders, plots, tables, docking) from them, so
all widgets follow it, and preserves metrics. Single colors can still be
overridden through `GetStyle()` afterwards. Start from `Palette::Dark()` or
`Palette::Light()`.

```cpp
ui::Palette palette = ui::Palette::Dark();
palette.Primary = {0.1f, 0.7f, 0.5f, 1};
palette.Secondary = {0.9f, 0.5f, 0.1f, 1};
auto styler = app.GetStyler();
styler.SetPalette(palette);
app.SetStyler(styler);
```


`Application::SetFont(path, size)` makes a TrueType/OpenType file the default
font and `ResetFont()` restores the built-in one. The file must be readable
(`std::invalid_argument` otherwise) and the size positive. Like styles, the
font is loaded at the next frame boundary, survives `SetStyler()` and is
render-thread-only; a later load failure is reported through the error handler.

### Concrete widgets (`cpptoolkit/ui/widgets/widgets.h`)

- `TextWidget` — read-only text, polls a bound `ObservableProperty<std::string>`.
- `TextBoxWidget` — single-line editable text with two-way binding to `ObservableProperty<std::string>` and an optional `OnTextChanged` callback.
- `FilePathWidget` — path text box plus a "Browse..." button opening a native Open/Save/Folder dialog (`FilePathMode`, optional `FileFilter`s). A cancelled dialog leaves the property unchanged; failures are exposed through `LastError()`.
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
| `DateTimeWidgets.h` | `DatePickerWidget` | `Date` (`std::chrono::year_month_day`), `OnDateChanged` |
| `DateTimeWidgets.h` | `TimePickerWidget` | `TimeOfDay` (`std::chrono::seconds` since midnight), `OnTimeChanged` |
| `DateTimeWidgets.h` | `DateTimePickerWidget` | `DateTime` (civil date + time), `OnDateTimeChanged` |
| `FeedbackWidgets.h` | `ProgressBarWidget`, `SpinnerWidget`, `BadgeWidget` | read-only progress, busy flag, status string |
| `FeedbackWidgets.h` | `NotificationWidget` | message and open flag, `OnDismissed` |
| `FeedbackWidgets.h` | `ImageWidget` | borrowed `ImTextureID` property; caller owns the texture |
| `RenderTextureWidget.h` | `RenderTextureWidget` | owned raylib framebuffer, `OnRenderTexture`, ambient ViewModel |
| `EditorLayoutWidget.h` | `EditorLayoutWidget` | five dockable panels, ambient ViewModel, `ResetLayout()` |
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

## Keyboard shortcuts

`Application::Shortcuts()` returns a `ShortcutManager` processed once per frame before the widget tree draws. Register a `ShortcutInfo` (unique id, description, `ImGuiKeyChord` such as `ImGuiMod_Ctrl | ImGuiKey_S`, action, optional `CanExecute`, `Repeat`, `AllowWhileTyping`) or bind a borrowed `mvvm::Command` with `Register(id, chord, command)`. Chords are unique (conflicts throw); `Rebind()`, `Unregister()`, `Find()`, `FindByChord()` and `Text()` (e.g. "Ctrl+S", for menus) support user remapping and help screens. Global routing lets focused items keep priority (text-field Ctrl+Z); chords without Ctrl/Alt/Super are ignored while typing unless `AllowWhileTyping`. Set `Enabled = false` while capturing a new chord. The demo registers Ctrl+Z / Ctrl+Y for undo/redo this way.

## Concept

ImGui itself stays immediate-mode under the hood (it must be called every frame), but the **structure** of the UI becomes retained: you build the `Panel`/`Widget` tree once, bind widgets to `cpptoolkit::mvvm::ObservableProperty<T>` instances, and just call `Draw()` in the render loop. Widgets read bound properties every frame (`Get()`), which is thread-safe by construction — see the `mvvm` module's thread-safety notes.

## Optional sub-functionality

Only the explicit `ui-net-widgets` feature / `CPPTOOLKIT_BUILD_UI_NET_WIDGETS`
option defines `CPPTOOLKIT_UI_HAS_NET_WIDGETS` and adds the net dependency.
Enabling `ui` and `net` separately does not activate this combined feature.

## Dependencies

- `cpptoolkit::platform` (private; OS detection macros)
- `cpptoolkit::mvvm`
- `imgui` (vcpkg feature `docking-experimental`)
- `implot`
- `nativefiledialog-extended` (private; native file dialogs, Zlib license)

`FileDialog` (`cpptoolkit/ui/FileDialog.h`) exposes `OpenFile`, `OpenFiles`,
`SaveFile` and `PickFolder` returning UTF-8 paths. These are modal and block
the calling thread; macOS requires the main thread. The `*Async` variants
(`OpenFileAsync`, ...) run the dialog on a worker thread and return a
`PendingFileDialog` to poll with `IsReady()` and `Take()`; `FilePathWidget` uses
them so rendering never stalls. On macOS async calls complete synchronously
(Cocoa needs the main thread). Call them from the render thread, which supplies
the parent window. Cancelling yields `nullopt`/empty, failures throw
`std::runtime_error` (async: `FileDialogResult::error`).
On Windows and macOS the dialog is parented to the application window; on Linux
this works under X11 and Wayland when the X11/Wayland development headers are
found at configure time (vcpkg builds GLFW with Wayland support on Linux),
otherwise dialogs open unparented. On Linux the
XDG desktop portal (D-Bus) is used, so a portal implementation must be running.
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

### Date and time pickers

```cpp
mvvm::ObservableProperty<ui::Date> date{
    nullptr, "Date", {std::chrono::year{2026}, std::chrono::October, std::chrono::day{10}}};
mvvm::ObservableProperty<ui::TimeOfDay> time{nullptr, "Time", std::chrono::seconds{12 * 3600}};
mvvm::ObservableProperty<ui::DateTime> dateTime{nullptr, "DateTime", {date.Get(), time.Get()}};
panel.Add<ui::DatePickerWidget>("Date", date).OnDateChanged = [](ui::Date selected) {
    // React to a user-selected calendar day.
};
panel.Add<ui::TimePickerWidget>("Time", time);
panel.Add<ui::DateTimePickerWidget>("Date and time", dateTime);
```

The calendar uses Monday-first weeks, an ISO `YYYY-MM-DD` preview, previous/next
month buttons and direct month/year selection. Selecting a day closes it;
browsing months does not alter the bound date. Valid Gregorian dates in years
1-9999 are supported, including leap years. Time is 24-hour `HH:MM:SS`, stored
as seconds since midnight in [0, 86399]. Edits and step buttons clamp each
component independently, without carrying or wrapping; leap seconds are excluded.
`DateTime` combines `date` and `time` in one property, committed atomically before
`OnDateTimeChanged`. These are **civil values, not timestamps**: no time zone,
UTC offset or daylight-saving conversion is inferred.

All three expose `readOnly`, honor `Visible`, and support unbound constructors
with a local initial value and optional callback. Invalid initial values throw
`std::invalid_argument`; invalid programmatic values show a diagnostic without
normalizing or notifying. Callbacks follow the binding rules above.

### Application

`examples/main.cpp` and `examples/TestUI.h/.cpp` (built with
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

`RaylibRenderingTests` also exercises real GPU framebuffer content, flipped UVs,
frame order and exceptional teardown. On Linux it requires an X11 display and
skips when none is set; run with `xvfb-run -a ctest --test-dir <build-dir> -R
RaylibRenderingTests --output-on-failure` for virtual-display validation.
