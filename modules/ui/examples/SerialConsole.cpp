#include "SerialConsole.h"

#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)

#include <chrono>
#include <cstdint>

namespace demo {

using namespace cpptoolkit;

SerialConsoleViewModel::~SerialConsoleViewModel() {
    reading_ = false;
    if (reader_.joinable()) reader_.join();
    transport_.Disconnect();
}

void SerialConsoleViewModel::Connect() {
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

void SerialConsoleViewModel::Send(const std::string& message) {
    if (!Connected.Get()) return;
    ui::Application::RunAsync([weak = weak_from_this(), message] {
        if (auto self = weak.lock()) {
            const auto written = self->transport_.Write(
                reinterpret_cast<const std::uint8_t*>(message.data()), message.size());
            if (written != message.size()) self->Log.Set("Incomplete serial write");
        }
    });
}

SerialPanel::SerialPanel() : ui::Panel("Serial Console") { SetViewModel(model_.get()); }

void SerialPanel::OnBuild() {
    Add<ui::ButtonWidget>("Connect", model_->ConnectCommand);
    Add<ui::ButtonWidget>("Send ping", model_->SendCommand);
    Add<ui::TextWidget>(model_->Log);
    auto& error = Add<ui::DialogWidget>("Serial error", model_->ErrorOpen);
    error.Add<ui::TextWidget>(model_->Log);
    error.Add<ui::ButtonWidget>("Close", model_->CloseError);
}

} // namespace demo

#endif
