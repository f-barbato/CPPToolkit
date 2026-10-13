#pragma once

// Serial console example: only built when the net module is enabled.
#if defined(CPPTOOLKIT_UI_DEMO_HAS_NET)

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include <cpptoolkit/mvvm/mvvm.h>
#include <cpptoolkit/net/net.h>
#include <cpptoolkit/ui/ui.h>

namespace demo {

/** @brief ViewModel of the serial console; the transport backend is currently a placeholder. */
class SerialConsoleViewModel : public cpptoolkit::mvvm::ObservableObject,
                               public std::enable_shared_from_this<SerialConsoleViewModel> {
public:
    cpptoolkit::mvvm::ObservableProperty<std::string> Log{this, "Log", "Serial transport is currently a placeholder"};
    cpptoolkit::mvvm::ObservableProperty<bool> Connected{this, "Connected", false};
    cpptoolkit::mvvm::ObservableProperty<bool> ErrorOpen{this, "ErrorOpen", false};
    cpptoolkit::mvvm::Command ConnectCommand{[this] { Connect(); }};
    cpptoolkit::mvvm::Command SendCommand{[this] { Send("ping\n"); }, [this] { return Connected.Get(); }};
    cpptoolkit::mvvm::Command CloseError{[this] { ErrorOpen.Set(false); }};

    ~SerialConsoleViewModel() override;

private:
    void Connect();
    void Send(const std::string& message);

    cpptoolkit::net::SerialTransport transport_{"COM3", 115200};
    std::thread reader_;
    std::atomic<bool> reading_{false};
    std::atomic<bool> connecting_{false};
};

/** @brief Window with Connect/Send buttons and the console log. */
class SerialPanel : public cpptoolkit::ui::Panel {
public:
    SerialPanel();

protected:
    void OnBuild() override;

private:
    std::shared_ptr<SerialConsoleViewModel> model_ = std::make_shared<SerialConsoleViewModel>();
};

} // namespace demo

#endif
