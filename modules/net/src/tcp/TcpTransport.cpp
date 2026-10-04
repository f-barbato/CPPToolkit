#include "cpptoolkit/net/tcp/TcpTransport.h"

namespace cpptoolkit::net {

TcpTransport::TcpTransport(std::string host, std::uint16_t port) : host_(std::move(host)), port_(port) {}

bool TcpTransport::Connect() {
    // TODO: implement platform-specific socket I/O (BSD sockets/WinSock).
    connected_ = false;
    return connected_;
}

void TcpTransport::Disconnect() { connected_ = false; }

bool TcpTransport::IsConnected() const { return connected_; }

std::size_t TcpTransport::Read(std::uint8_t* /*buffer*/, std::size_t /*maxSize*/) { return 0; }

std::size_t TcpTransport::Write(const std::uint8_t* /*data*/, std::size_t /*size*/) { return 0; }

} // namespace cpptoolkit::net
