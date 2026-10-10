#pragma once

#include <string>

#include "cpptoolkit/net/ITransport.h"

/**
 * @file SerialTransport.h
 * @brief Serial transport interface and placeholder implementation.
 */

namespace cpptoolkit::net {

/**
 * @brief Describes a serial port transport.
 *
 * Current implementation is a placeholder: Connect() reports failure,
 * IsConnected() reports false, and Read()/Write() return zero. No serial I/O
 * is performed.
 */
class CPPTOOLKIT_NET_EXPORT SerialTransport : public ITransport {
public:
    /**
     * @brief Constructs a transport configuration.
     * @param portName Serial port name, stored for future platform I/O.
     * @param baudRate Baud rate, stored for future platform I/O.
     */
    SerialTransport(std::string portName, int baudRate);

    /**
     * @brief Currently performs no I/O and always returns false.
     * @return Always false.
     */
    bool Connect() override;
    /** @brief Sets the reported connection state to false. */
    void Disconnect() override;
    /**
     * @brief Currently always returns false.
     * @return Always false.
     */
    [[nodiscard]] bool IsConnected() const override;

    /**
     * @brief Currently performs no I/O and returns zero.
     * @param buffer Unused destination buffer.
     * @param maxSize Unused maximum read size.
     * @return Always zero.
     */
    std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) override;
    /**
     * @brief Currently performs no I/O and returns zero.
     * @param data Unused source data.
     * @param size Unused data size.
     * @return Always zero.
     */
    std::size_t Write(const std::uint8_t* data, std::size_t size) override;

private:
    std::string portName_;
    int baudRate_;
    bool connected_ = false;
};

} // namespace cpptoolkit::net
