#pragma once

#include <cstddef>
#include <cstdint>

namespace cpptoolkit::net {

// Common abstraction over any communication transport (serial, TCP, BLE, ...),
// so the rest of the toolkit (e.g. mvvm ViewModels, ui widgets) does not need
// to know which concrete transport is in use.
class ITransport {
public:
    virtual ~ITransport() = default;

    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;
    [[nodiscard]] virtual bool IsConnected() const = 0;

    virtual std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) = 0;
    virtual std::size_t Write(const std::uint8_t* data, std::size_t size) = 0;
};

} // namespace cpptoolkit::net
