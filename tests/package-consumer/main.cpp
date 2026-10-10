#include <cstdint>
#include <stdexcept>

#include <cpptoolkit/platform/Platform.h>
#include <cpptoolkit/struct/RingBuffer.h>
#include <cpptoolkit/algo/Algo.h>
#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/net/net.h>

#ifdef CPPTOOLKIT_CONSUMER_UI
#include <cpptoolkit/ui/ui.h>
#include <imgui_impl_raylib.h>
#endif

int main() {
    using namespace cpptoolkit;
    const auto require = [](bool condition) {
        if (!condition) throw std::runtime_error("Installed package consumer check failed");
    };
    require(platform::GetCurrentOS() != platform::OperatingSystem::Unknown);
    structs::RingBuffer<int, 4> values;
    values.Push(7);
    require(values.At(0) == 7);
    bool dispatched = false;
    mvvm::Dispatcher::Main().Post([&] { dispatched = true; });
    mvvm::Dispatcher::Main().ProcessPending();
    require(dispatched);
    auto& worker = mvvm::NotificationWorker::Instance();
    (void)worker;
    net::SerialTransport serial("test", 9600);
    net::TcpTransport tcp("localhost", 1234);
    require(!serial.Connect() && !tcp.Connect());

#ifdef CPPTOOLKIT_CONSUMER_UI
    // Contexts are created by the executable; widgets execute in another shared library.
    ImGui::CreateContext();
    ImPlot::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.0f / 60;
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    require(pixels != nullptr && width > 0 && height > 0);

    mvvm::ObservableProperty<std::string> text{nullptr, "Text", "Installed consumer"};
    mvvm::ObservableProperty<float> signal{nullptr, "Signal", 0.5f};
    mvvm::ObservableProperty<bool> checked{nullptr, "Checked", true};
    mvvm::ObservableProperty<ui::Date> date{nullptr, "Date", ui::DateTime{}.date};
    mvvm::ObservableProperty<ui::TimeOfDay> time{nullptr, "Time", ui::TimeOfDay{0}};
    mvvm::ObservableProperty<ui::DateTime> dateTime{nullptr, "DateTime", ui::DateTime{}};
    ui::Panel panel;
    ui::RenderTextureWidget framebuffer("Framebuffer API", 256, 240);
    framebuffer.SetDisplaySize(ImVec2(512, 480));
    require(framebuffer.GetDisplaySize().x == 512);
    framebuffer.RenderEnabled = false;
    framebuffer.Render();
    panel.Add<ui::TextBoxWidget>("Text", text);
    panel.Add<ui::CheckBoxWidget>("Enabled", checked);
    panel.Add<ui::DatePickerWidget>("Date", date);
    panel.Add<ui::TimePickerWidget>("Time", time);
    panel.Add<ui::DateTimePickerWidget>("Date/time", dateTime);
    panel.Add<ui::PlotLineWidget>("Signal", signal);
    panel.Build();
    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame();
        ImGui::Begin("Consumer");
        panel.Draw();
        ImGui::End();
        ImGui::Render();
        require(ImGui::GetDrawData() != nullptr);
    }
    panel.Destroy();
    // Verify that the low-level bridge API is exported by rlImGui too.
    auto* volatile bridgeInit = &ImGui_ImplRaylib_Init;
    auto* volatile bridgeShutdown = &ImGui_ImplRaylib_Shutdown;
    require(bridgeInit != nullptr && bridgeShutdown != nullptr);
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ui::Application application;
    application.SetTitle("Installed shared consumer").SetInitialSize(800, 600);
#endif
}
