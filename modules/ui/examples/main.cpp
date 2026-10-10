#include <cmath>
#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/ui/ui.h>
#include "TestUI.h"

#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)
#include <atomic>
#include <chrono>
#include <thread>
#include <cpptoolkit/net/net.h>
#endif

using namespace cpptoolkit;

namespace {

class DemoViewModel : public mvvm::ObservableObject {
public:
    mvvm::ObservableProperty<float> SensorValue{this, "SensorValue", 0};
    mvvm::ObservableProperty<float> Amplitude{this, "Amplitude", 1};
    mvvm::ObservableProperty<std::string> Status{this, "Status", "Running"};
    mvvm::ObservableProperty<std::string> TextInput{this, "TextInput", ""};
    TestUIState Gallery;
    mvvm::Command ResetCommand{[this] {
        phase_ = 0;
        Status.Set("Reset");
    }};
    void Tick(float dt) {
        phase_ += dt;
        SensorValue.Set(Amplitude.Get() * std::sin(phase_));
        Gallery.Tick(dt);
    }
private:
    float phase_ = 0;
};

class DemoPanel : public ui::Panel {
public:
    DemoPanel() : ui::Panel("TestUI") { SetFlags(ImGuiWindowFlags_MenuBar); }
protected:
    void OnBuild() override {
        auto* vm = GetViewModel<DemoViewModel>();
        if (!vm) return;
        Add<ui::TextWidget>(vm->Status);
        Add<ui::SliderFloatWidget>("Amplitude", vm->Amplitude, 0, 5);
        Add<ui::ButtonWidget>("Reset phase", vm->ResetCommand);
        Add<ui::PlotLineWidget>("Sensor value", vm->SensorValue);
        Add<ui::TextBoxWidget>("Text input", vm->TextInput);
        Add<ui::TextBoxWidget>("Text input (read-only)", vm->TextInput).readOnly = true;
        BuildTestUIWidgets(*this, vm->Gallery);
    }
};

#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)
class SerialConsoleViewModel : public mvvm::ObservableObject,
                               public std::enable_shared_from_this<SerialConsoleViewModel> {
public:
    mvvm::ObservableProperty<std::string> Log{this, "Log", "Serial transport is currently a placeholder"};
    mvvm::ObservableProperty<bool> Connected{this, "Connected", false};
    mvvm::ObservableProperty<bool> ErrorOpen{this, "ErrorOpen", false};
    mvvm::Command ConnectCommand{[this] { Connect(); }};
    mvvm::Command SendCommand{[this] { Send("ping\n"); }, [this] { return Connected.Get(); }};
    mvvm::Command CloseError{[this] { ErrorOpen.Set(false); }};

    ~SerialConsoleViewModel() override {
        reading_ = false;
        if (reader_.joinable()) reader_.join();
        transport_.Disconnect();
    }

private:
    void Connect() {
        if (Connected.Get() || connecting_.exchange(true)) return;
        // Queued tasks must not outlive their model; weak captures also cover shutdown.
        ui::Application::RunAsync([weak = weak_from_this()] {
            auto self = weak.lock();
            if (!self) return;
            if (!self->transport_.Connect()) {
                self->Log.Set("Connection failed (serial backend is a placeholder)");
                self->connecting_ = false;
                ui::Application::Dispatch([weak] {
                    if (auto model = weak.lock()) model->ErrorOpen.Set(true);
                });
                return;
            }
            self->Connected.Set(true);
            self->Log.Set("Connected");
            self->reading_ = true;
            self->reader_ = std::thread([model = self.get(), weak] {
                std::uint8_t buffer[256];
                while (model->reading_) {
                    const auto count = model->transport_.Read(buffer, sizeof(buffer));
                    if (count > 0) {
                        std::string chunk(reinterpret_cast<char*>(buffer), count);
                        ui::Application::RunAsync([weak, chunk = std::move(chunk)] {
                            if (auto live = weak.lock()) live->Log.Set(live->Log.Get() + chunk);
                        });
                    } else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
            });
            self->connecting_ = false;
        });
    }
    void Send(const std::string& message) {
        if (!Connected.Get()) return;
        ui::Application::RunAsync([weak = weak_from_this(), message] {
            if (auto self = weak.lock()) {
                const auto written = self->transport_.Write(
                    reinterpret_cast<const std::uint8_t*>(message.data()), message.size());
                if (written != message.size()) self->Log.Set("Incomplete serial write");
            }
        });
    }
    net::SerialTransport transport_{"COM3", 115200};
    std::thread reader_;
    std::atomic<bool> reading_{false};
    std::atomic<bool> connecting_{false};
};

class SerialPanel : public ui::Panel {
public:
    SerialPanel() : ui::Panel("Serial Console") { SetViewModel(model_.get()); }
protected:
    void OnBuild() override {
        Add<ui::ButtonWidget>("Connect", model_->ConnectCommand);
        Add<ui::ButtonWidget>("Send ping", model_->SendCommand);
        Add<ui::TextWidget>(model_->Log);
        auto& error = Add<ui::DialogWidget>("Serial error", model_->ErrorOpen);
        error.Add<ui::TextWidget>(model_->Log);
        error.Add<ui::ButtonWidget>("Close", model_->CloseError);
    }
private:
    std::shared_ptr<SerialConsoleViewModel> model_ = std::make_shared<SerialConsoleViewModel>();
};
#endif

class EditorPanel : public ui::EditorLayoutWidget {
public:
    EditorPanel() : ui::EditorLayoutWidget("Editor", ui::EditorLayout::LeftFullHeight) {}
protected:
    void OnBuild() override {
        Left().Add<ui::TextWidget>("Project / devices");
        Right().Add<ui::TextWidget>("Properties");
        Top().Add<ui::TextWidget>("CPPToolkit editor workspace");
        Bottom().Add<ui::TextWidget>("Output / diagnostics");
        Add<DemoPanel>();
        auto& screen = Center();
        auto* vm = GetViewModel<DemoViewModel>();
        if (!vm) throw std::logic_error("Framebuffer demo requires DemoViewModel");
        Right().Add<ui::SliderFloatWidget>("Amplitude", vm->Amplitude, 0, 5);
        Top().Add<ui::ButtonWidget>("Reset phase", vm->ResetCommand);
        auto& controls = Add<ui::Panel>("Editor layout controls");
        controls.Add<ui::TextWidget>("Layout presets");
        controls.Add<ui::ButtonWidget>("Full-height left", [this] {
            SetLayout(ui::EditorLayout::LeftFullHeight);
        });
        controls.Add<ui::ButtonWidget>("Full-height right", [this] {
            SetLayout(ui::EditorLayout::RightFullHeight);
        });
        controls.Add<ui::ButtonWidget>("Full-height sidebars", [this] {
            SetLayout(ui::EditorLayout::SidebarsFullHeight);
        });
        controls.Add<ui::ButtonWidget>("Full-width top and bottom", [this] {
            SetLayout(ui::EditorLayout::TopBottomFullWidth);
        });
        controls.Add<ui::TextWidget>("Show / hide panels");
        for (const auto& [label, region] : {
                 std::pair{"Toggle left", ui::EditorRegion::Left},
                 std::pair{"Toggle right", ui::EditorRegion::Right},
                 std::pair{"Toggle top", ui::EditorRegion::Top},
                 std::pair{"Toggle bottom", ui::EditorRegion::Bottom},
                 std::pair{"Toggle center", ui::EditorRegion::Center}}) {
            controls.Add<ui::ButtonWidget>(label, [this, region] {
                SetPanelVisible(region, !IsPanelVisible(region));
            });
        }
        controls.Add<ui::ButtonWidget>("Reset layout and show all panels", [this] { ResetLayout(); });
        Bottom().Add<ui::TextWidget>(vm->Status);
        screen.Add<ui::SliderFloatWidget>("Framebuffer amplitude", vm->Amplitude, 0, 5);
        auto& framebuffer = screen.Add<ui::RenderTextureWidget>("Shared ViewModel rendering", 320, 240,
            [](ui::RenderTextureWidget& widget) {
                auto* model = widget.GetViewModel<DemoViewModel>();
                if (!model) throw std::logic_error("Framebuffer widget requires DemoViewModel");
                const float value = model->SensorValue.Get();
                DrawRectangle(0, 0, 320, 120, DARKBLUE);
                DrawCircle(160 + static_cast<int>(value * 25), 120, 20, ORANGE);
                DrawText("raylib + shared MVVM", 12, 200, 18, RAYWHITE);
            });
        framebuffer.SetDisplaySize(ImVec2(640, 480));
#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)
        Add<SerialPanel>();
#endif
    }
};

class EditorView : public ui::View<DemoViewModel> {
protected:
    void OnBuild() override { Add<EditorPanel>(); }
public:
    void Render() override {
        if (!RenderEnabled) return;
        _viewModel->Tick(GetFrameTime());
        Widget::Render();
    }
};

} // namespace

int main() {
    auto& app = ui::Application::Create<EditorView>();
    app.SetTitle("CPPToolkit TestUI")
       .SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI)
       .SetInitialSize(1200, 900)
       .SetMinSize(800, 600)
       .SetTargetFPS(144)
       .SetBackgroundColor(DARKGRAY)
       .SetDarkTheme(true);
    return app.Run();
}
