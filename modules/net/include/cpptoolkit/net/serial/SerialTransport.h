#pragma once

#include <string>

#include "cpptoolkit/net/ITransport.h"

namespace cpptoolkit::net {

// Serial (UART/USB-CDC) transport. Platform-specific I/O is implemented in
// SerialTransport.cpp (currently a stub — TODO: implement termios/WinAPI I/O).
class SerialTransport : public ITransport {
public:
    SerialTransport(std::string portName, int baudRate);

    bool Connect() override;
    void Disconnect() override;
    [[nodiscard]] bool IsConnected() const override;

    std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) override;
    std::size_t Write(const std::uint8_t* data, std::size_t size) override;

private:
    std::string portName_;
    int baudRate_;
    bool connected_ = false;
};

} // namespace cpptoolkit::net
