#pragma once

#include <cstdint>
#include <string>

#include "cpptoolkit/net/ITransport.h"

namespace cpptoolkit::net {

// TCP/IP transport. Socket I/O is implemented in TcpTransport.cpp (currently
// a stub — TODO: implement BSD sockets/WinSock I/O).
class TcpTransport : public ITransport {
public:
    TcpTransport(std::string host, std::uint16_t port);

    bool Connect() override;
    void Disconnect() override;
    [[nodiscard]] bool IsConnected() const override;

    std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) override;
    std::size_t Write(const std::uint8_t* data, std::size_t size) override;

private:
    std::string host_;
    std::uint16_t port_;
    bool connected_ = false;
};

} // namespace cpptoolkit::net
