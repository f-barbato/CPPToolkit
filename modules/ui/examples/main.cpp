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

#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)
#include <atomic>
#include <thread>
#include <cpptoolkit/net/net.h>
#endif

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

#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)

// Demonstrates routing BOTH directions of a serial connection through the
// notification system (Application::RunAsync -> mvvm::NotificationWorker)
// instead of calling blocking I/O or Set()/Notify() directly from whichever
// thread triggered it:
//  - incoming bytes: read on a dedicated background thread (ReadLoop), which
//    only ever blocks on transport_.Read(); publishing the result is handed
//    off via RunAsync() so a slow OnChanged() handler never adds latency to
//    the read loop itself.
//  - outgoing bytes: Send() is invoked from the render thread (a button
//    click), but the actual transport_.Write() runs via RunAsync() too, so a
//    busy/slow device can never stall a frame.
//  - Application::Dispatch() is reserved for the one case that truly must
//    run on the render thread: reacting to a connection failure by opening
//    an ImGui popup.
class SerialConsoleViewModel : public mvvm::ObservableObject {
public:
    mvvm::ObservableProperty<std::string> Log{this, "Log", "(disconnected)"};
    mvvm::ObservableProperty<bool> Connected{this, "Connected", false};

    mvvm::Command ConnectCommand{[this] { Connect(); }};
    mvvm::Command SendCommand{[this] { Send("ping\n"); }};

    ~SerialConsoleViewModel() override { Disconnect(); }

private:
    void Connect() {
        if (Connected.Get()) return;

        // transport_.Connect() may block (device enumeration/handshake):
        // never call it directly from a widget callback (render thread).
        ui::Application::RunAsync([this] {
            if (!transport_.Connect()) {
                ui::Application::Dispatch([this] { ImGui::OpenPopup("Serial error"); });
                Log.Set("Connection failed");
                return;
            }

            Connected.Set(true);
            Log.Set("Connected");

            reading_ = true;
            reader_ = std::thread([this] { ReadLoop(); });
        });
    }

    void Disconnect() {
        reading_ = false;
        if (reader_.joinable()) reader_.join();
        transport_.Disconnect();
        Connected.Set(false);
    }

    void Send(const std::string& message) {
        if (!Connected.Get()) return;
        ui::Application::RunAsync([this, message] {
            transport_.Write(reinterpret_cast<const std::uint8_t*>(message.data()), message.size());
        });
    }

    void ReadLoop() {
        std::uint8_t buffer[256];
        while (reading_) {
            std::size_t n = transport_.Read(buffer, sizeof(buffer));
            if (n > 0) {
                std::string chunk(reinterpret_cast<char*>(buffer), n);
                // Hand the received chunk to the notification worker rather
                // than calling Log.Set() directly here, keeping the read
                // loop itself free of any OnChanged()-handler latency.
                ui::Application::RunAsync([this, chunk] { Log.Set(Log.Get() + chunk); });
            }
        }
    }

    net::SerialTransport transport_{"COM3", 115200};
    std::thread reader_;
    std::atomic<bool> reading_ = false;
};

// Standalone ViewModel: owns its own SerialConsoleViewModel instead of
// relying on the ambient cascade from the root (which carries DemoViewModel),
// by calling SetViewModel() itself in the constructor - Widget::Add() only
// cascades a parent's ViewModel into children that don't already have one.
class SerialPanel : public ui::Panel {
public:
    SerialPanel() : ui::Panel("Serial Console") {
        SetViewModel(viewModel_.get());
    }

protected:
    void OnBuild() override {
        SetFlags(ImGuiWindowFlags_NoCollapse);

        Add<ui::ButtonWidget>("Connect", viewModel_->ConnectCommand);
        Add<ui::ButtonWidget>("Send \"ping\"", viewModel_->SendCommand);
        Add<ui::TextWidget>(viewModel_->Log);
    }

private:
    std::shared_ptr<SerialConsoleViewModel> viewModel_ = std::make_shared<SerialConsoleViewModel>();
};

#endif // CPPTOOLKIT_UI_DEMO_HAS_NET

// Hosts DemoPanel inside an ImGui dockspace.
class EditorPanel : public ui::DockedPanel {
protected:
    void OnBuild() override {
        Add<DemoPanel>();
#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)
        Add<SerialPanel>();
#endif
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
