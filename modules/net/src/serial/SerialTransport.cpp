#include "cpptoolkit/net/serial/SerialTransport.h"

namespace cpptoolkit::net {

SerialTransport::SerialTransport(std::string portName, int baudRate)
    : portName_(std::move(portName)), baudRate_(baudRate) {}

bool SerialTransport::Connect() {
    // TODO: implement platform-specific serial I/O (termios on POSIX, WinAPI on Windows).
    connected_ = false;
    return connected_;
}

void SerialTransport::Disconnect() { connected_ = false; }

bool SerialTransport::IsConnected() const { return connected_; }

std::size_t SerialTransport::Read(std::uint8_t* /*buffer*/, std::size_t /*maxSize*/) { return 0; }

std::size_t SerialTransport::Write(const std::uint8_t* /*data*/, std::size_t /*size*/) { return 0; }

} // namespace cpptoolkit::net
