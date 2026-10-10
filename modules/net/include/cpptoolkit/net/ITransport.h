#pragma once
#include <cpptoolkit/net/Export.h>

#include <cstddef>
#include <cstdint>

/**
 * @file ITransport.h
 * @brief Common interface for communication transports.
 */

namespace cpptoolkit::net {

/**
 * @brief Abstract interface for byte-oriented communication transports.
 *
 * Concrete transport implementations determine connection and I/O details.
 */
class CPPTOOLKIT_NET_EXPORT ITransport {
public:
    /** @brief Destroys the transport through the interface. */
    virtual ~ITransport() = default;

    /**
     * @brief Attempts to establish the transport connection.
     * @return Whether the connection was established, as defined by the
     * concrete implementation.
     */
    virtual bool Connect() = 0;
    /** @brief Disconnects the transport, as defined by its implementation. */
    virtual void Disconnect() = 0;
    /**
     * @brief Reports whether the concrete transport is connected.
     * @return Whether the concrete transport reports an active connection.
     */
    [[nodiscard]] virtual bool IsConnected() const = 0;

    /**
     * @brief Reads up to maxSize bytes.
     * @param buffer Destination for received bytes.
     * @param maxSize Maximum number of bytes to place in buffer.
     * @return Number of bytes reported read by the implementation.
     */
    virtual std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) = 0;
    /**
     * @brief Writes up to size bytes.
     * @param data Source bytes to write.
     * @param size Number of bytes offered to the implementation.
     * @return Number of bytes reported written by the implementation.
     */
    virtual std::size_t Write(const std::uint8_t* data, std::size_t size) = 0;
};

} // namespace cpptoolkit::net
