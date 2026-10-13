#include "DemoPanels.h"

#include <stdexcept>
#include <vector>

#include "ControlsPanel.h"
#include "SerialConsole.h"
#include "TestUI.h"

namespace demo {

using namespace cpptoolkit;

LiveDemoPanel::LiveDemoPanel() : ui::Panel("Live demo") {}

void LiveDemoPanel::OnBuild() {
    auto* vm = GetViewModel<DemoViewModel>();
    if (!vm) return;
    RegisterUndoShortcuts(*vm);
    Add<ui::TextWidget>(vm->Status);
    auto& tabs = Add<ui::TabBarWidget>("Live demo sections", selectedTab_);

    auto& telemetry = tabs.AddTab("Telemetry");
    telemetry.Add<ui::SliderFloatWidget>("Amplitude", vm->Amplitude, 0.0f, 5.0f).History = &vm->History;
    telemetry.Add<ui::ButtonWidget>("Reset phase", vm->ResetCommand);
    telemetry.Add<ui::PlotLineWidget>("Sensor value", vm->SensorValue);

    auto& editing = tabs.AddTab("Editing");
    editing.Add<ui::ButtonWidget>("Undo (Ctrl+Z)", vm->History.UndoCommand);
    editing.Add<ui::ButtonWidget>("Redo (Ctrl+Y)", vm->History.RedoCommand);
    editing.Add<ui::TextWidget>(vm->History.UndoText);
    editing.Add<ui::TextBoxWidget>("Text input", vm->TextInput).History = &vm->History;
    editing.Add<ui::TextBoxWidget>("Text input (read-only)", vm->TextInput).readOnly = true;
}

void LiveDemoPanel::RegisterUndoShortcuts(DemoViewModel& vm) {
    auto* app = GetApplication();
    if (!app) return;
    auto& shortcuts = app->Shortcuts();
    if (shortcuts.Find("edit.undo")) return;
    shortcuts.Register("edit.undo", ImGuiMod_Ctrl | ImGuiKey_Z, vm.History.UndoCommand, "Undo");
    shortcuts.Register("edit.redo", ImGuiMod_Ctrl | ImGuiKey_Y, vm.History.RedoCommand, "Redo");
    shortcuts.Register("edit.redo.alt", ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, vm.History.RedoCommand,
                       "Redo");
}

GalleryPanel::GalleryPanel() : ui::Panel("TestUI") { SetFlags(ImGuiWindowFlags_MenuBar); }

void GalleryPanel::OnBuild() {
    auto* vm = GetViewModel<DemoViewModel>();
    if (!vm) return;
    BuildTestUIWidgets(*this, vm->Gallery);
}

EditorPanel::EditorPanel() : ui::EditorLayoutWidget("Editor", ui::EditorLayout::LeftFullHeight) {}

void EditorPanel::OnBuild() {
    auto* vm = GetViewModel<DemoViewModel>();
    if (!vm) throw std::logic_error("Editor demo requires DemoViewModel");
    BuildRegions(*vm);
    auto& live = Add<LiveDemoPanel>();
    auto& gallery = Add<GalleryPanel>();
    Add<ControlsPanel>(*this, std::vector<ui::Panel*>{&Left(), &Right(), &Top(), &Bottom(), &Center(), &live,
                                                      &gallery});
#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)
    Add<SerialPanel>();
#endif
}

void EditorPanel::BuildRegions(DemoViewModel& vm) {
    Left().Add<ui::TextWidget>("Project / devices");
    Right().Add<ui::TextWidget>("Properties");
    Right().Add<ui::SliderFloatWidget>("Amplitude", vm.Amplitude, 0.0f, 5.0f).History = &vm.History;
    Top().Add<ui::TextWidget>("CPPToolkit editor workspace");
    Top().Add<ui::ButtonWidget>("Reset phase", vm.ResetCommand);
    Bottom().Add<ui::TextWidget>("Output / diagnostics");
    Bottom().Add<ui::TextWidget>(vm.Status);
    Center().Add<ui::SliderFloatWidget>("Framebuffer amplitude", vm.Amplitude, 0.0f, 5.0f);
    auto& framebuffer = Center().Add<ui::RenderTextureWidget>(
        "Shared ViewModel rendering", 320, 240, [](ui::RenderTextureWidget& widget) {
            auto* model = widget.GetViewModel<DemoViewModel>();
            if (!model) throw std::logic_error("Framebuffer widget requires DemoViewModel");
            const float value = model->SensorValue.Get();
            DrawRectangle(0, 0, 320, 120, DARKBLUE);
            DrawCircle(160 + static_cast<int>(value * 25), 120, 20, ORANGE);
            DrawText("raylib + shared MVVM", 12, 200, 18, RAYWHITE);
        });
    framebuffer.SetDisplaySize(ImVec2(640, 480));
}

} // namespace demo
