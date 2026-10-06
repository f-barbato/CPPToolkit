// Example: raylib (window/rendering) + Dear ImGui (via rlImGui) + ImPlot,
// using cpptoolkit::ui's retained Widget/Panel tree bound to an
// cpptoolkit::mvvm ViewModel.
//
// Demonstrates:
//  - a Panel built once (DemoView), not recreated every frame
//  - two-way binding via SliderFloatWidget (writes back through Set())
//  - one-way binding via TextWidget/PlotLineWidget (polling Get() every frame)
//  - a Command bound to ButtonWidget

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

    // Called once per frame from the main loop (simulates a sensor reading
    // that, in a real tool, would instead come from a cpptoolkit::net
    // transport running on a background thread).
    void Tick(float dt) {
        phase_ += dt;
        SensorValue.Set(Amplitude.Get() * std::sin(phase_));
    }

private:
    float phase_ = 0.0f;
};

// Widget tree built once in the constructor; only Draw() runs every frame.
class DemoView : public ui::View<DemoViewModel> {
public:
    void Build() override {
        Add<ui::TextWidget>(_viewModel->Status);
        Add<ui::SliderFloatWidget>("Amplitude", _viewModel->Amplitude, 0.0f, 5.0f);
        Add<ui::ButtonWidget>("Reset phase", _viewModel->ResetCommand);
        Add<ui::PlotLineWidget>("Sensor value", _viewModel->SensorValue);

        BuildChildren();
    }

    void Draw() override {
        ImGui::Begin("Demo View");
        
        DrawChildren();
        
        ImGui::End();
    }
};

class DockedView : public ui::View<DemoViewModel> {
public:
    void Build() override {

        #ifdef IMGUI_HAS_DOCK
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        #endif


        Add<ui::TextWidget>(_viewModel->Status);
        Add<ui::SliderFloatWidget>("Amplitude", _viewModel->Amplitude, 0.0f, 5.0f);
        Add<ui::ButtonWidget>("Reset phase", _viewModel->ResetCommand);
        Add<ui::PlotLineWidget>("Sensor value", _viewModel->SensorValue);

        BuildChildren();
    }

    void Draw() override {

        #ifdef IMGUI_HAS_DOCK
        ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);
        #endif

        ImGui::ShowDemoWindow();

        ImGui::Begin("Docked View");
        
        DrawChildren();
        
        ImGui::End();
    }
};


} // namespace

int main() {
    
    auto& app = ui::Application::GetInstance<DockedView>();

    app.SetTitle("Docked View Example")
       .SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_UNDECORATED)
       .SetInitialSize(800, 600)
       .SetTargetFPS(144)
       .SetDarkTheme(true);

    return app.Run();
}
