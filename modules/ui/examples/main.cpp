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
class DemoView : public ui::Panel {
public:
    explicit DemoView(DemoViewModel& vm) {
        Add<ui::TextWidget>(vm.Status);
        Add<ui::SliderFloatWidget>("Amplitude", vm.Amplitude, 0.0f, 5.0f);
        Add<ui::ButtonWidget>("Reset phase", vm.ResetCommand);
        Add<ui::PlotLineWidget>("Sensor value", vm.SensorValue);
    }
};

} // namespace

int main() {
    /*InitWindow(900, 600, "CPPToolkit UI Demo (raylib + ImGui + ImPlot)");
    SetTargetFPS(60);

    rlImGuiSetup(true);
    ImPlot::CreateContext();

    DemoViewModel viewModel;
    DemoView view(viewModel);

    while (!WindowShouldClose()) {
        viewModel.Tick(GetFrameTime());

        BeginDrawing();
        ClearBackground(RAYWHITE);

        rlImGuiBegin();
        ImGui::Begin("MCU Debug Tool - Demo");
        view.Draw();
        ImGui::End();
        rlImGuiEnd();

        EndDrawing();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
    return 0;*/

    ui::Application app;
    return app.Run();
}
