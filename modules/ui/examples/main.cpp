// Example: raylib (window/rendering) + Dear ImGui (via rlImGui) + ImPlot,
// using cpptoolkit::ui's retained Widget/Panel tree, built via
// Application::GetInstance<T>() (factory/singleton per root-widget type) and
// a fluent setup builder.
//
// Demonstrates:
//  - a tree built once (EditorView -> EditorPanel -> DemoPanel), not
//    recreated every frame
//  - the ViewModel attached once at the root (View<DemoViewModel>) and
//    cascading automatically down to nested panels as an ambient, optional
//    binding context (DemoPanel fetches it via GetViewModel<T>(), with no
//    constructor threading)
//  - two-way binding via SliderFloatWidget (writes back through Set())
//  - one-way binding via TextWidget/PlotLineWidget (polling Get() every frame)
//  - a Command bound to ButtonWidget
//  - ImGui docking (DockedPanel) hosting a regular Panel

#include <cmath>

#include <raylib.h>
#include <rlImGui.h>

#include <imgui.h>
#include <implot.h>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/ui.h>

using namespace cpptoolkit;

namespace {

class DemoViewModel : public mvvm::ObservableObject {
public:
    mvvm::ObservableProperty<float> SensorValue{this, "SensorValue", 0.0f};
    mvvm::ObservableProperty<float> Amplitude{this, "Amplitude", 1.0f};
    mvvm::ObservableProperty<std::string> Status{this, "Status", "Running"};

    mvvm::Command ResetCommand{[this] {
        phase_ = 0.0f;
        Status.Set("Reset");
    }};

    // Called once per frame (see EditorView::Draw() below) to simulate a
    // sensor reading that, in a real tool, would instead come from a
    // cpptoolkit::net transport running on a background thread.
    void Tick(float dt) {
        phase_ += dt;
        SensorValue.Set(Amplitude.Get() * std::sin(phase_));
    }

private:
    float phase_ = 0.0f;
};

// No constructor parameter needed: the ViewModel is fetched lazily from the
// ambient binding context (set once on the root View and cascaded down
// automatically by Widget::Add), and binding stays fully optional - this
// panel simply does nothing if none is available.
class DemoPanel : public ui::Panel {
public:
    DemoPanel() : ui::Panel("Demo Panel") {}

protected:
    void OnBuild() override {

        this->SetFlags(ImGuiWindowFlags_NoCollapse);

        auto* viewModel = GetViewModel<DemoViewModel>();
        if (!viewModel) return;

        Add<ui::TextWidget>(viewModel->Status);
        Add<ui::ButtonWidget>("Click me", viewModel->ResetCommand);
        Add<ui::SliderFloatWidget>("Volume", viewModel->Amplitude, 0.0f, 5.0f);
        Add<ui::PlotLineWidget>("Sensor value", viewModel->SensorValue);
    }
};

// Hosts DemoPanel inside an ImGui dockspace.
class EditorPanel : public ui::DockedPanel {
protected:
    void OnBuild() override {
        Add<DemoPanel>();
    }
};

// Root widget: owns the DemoViewModel (created by View<T>) and ticks it once
// per frame before drawing, since Application::Run() no longer exposes a
// separate per-frame callback.
class EditorView : public ui::View<DemoViewModel> {
protected:
    void OnBuild() override {
        Add<EditorPanel>();
    }

public:
    void Draw() override {
        _viewModel->Tick(GetFrameTime());
        Widget::Draw();
    }
};

} // namespace

int main() {
    
    auto& app = ui::Application::Create<EditorView>();

    app.SetTitle("Docked View Example")
       .SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI)
       .SetInitialSize(900, 600)
       .SetTargetFPS(144)
       .SetBackgroundColor(DARKGRAY)
       .SetDarkTheme(true);

    return app.Run();
}
