#pragma once

#include <string>

#include <simpleble/Peripheral.h>

#include "cpptoolkit/net/ITransport.h"

/**
 * @file BleTransport.h
 * @brief Bluetooth Low Energy transport backed by SimpleBLE.
 */

namespace cpptoolkit::net {

/**
 * @brief Provides connection operations for a SimpleBLE peripheral.
 *
 * Connect() and connection-state queries use the supplied peripheral.
 * Read()/Write() are placeholders and currently return zero; they do not
 * transfer data because no GATT characteristic selection is implemented.
 *
 * This declaration is available when CPPTOOLKIT_NET_HAS_BLE is defined.
 * SimpleBLE's BUSL-1.1 licensing terms should be reviewed before distributing
 * binaries that link it.
 */
class BleTransport : public ITransport {
public:
    /**
     * @brief Constructs a transport from a SimpleBLE peripheral.
     * @param peripheral Peripheral handle to use for connection operations.
     */
    explicit BleTransport(SimpleBLE::Peripheral peripheral);

    /**
     * @brief Attempts to connect the peripheral when it is initialized.
     * @return False if the peripheral is uninitialized; otherwise returns the
     * peripheral's connection state after calling SimpleBLE connect().
     */
    bool Connect() override;
    /** @brief Disconnects an initialized peripheral when it is connected. */
    void Disconnect() override;
    /**
     * @brief Reports the peripheral connection state.
     * @return False for an uninitialized peripheral; otherwise the state
     * reported by SimpleBLE.
     */
    [[nodiscard]] bool IsConnected() const override;

    /**
     * @brief Currently does not read a GATT characteristic and returns zero.
     * @param buffer Unused destination buffer.
     * @param maxSize Unused maximum read size.
     * @return Always zero.
     */
    std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) override;
    /**
     * @brief Currently does not write a GATT characteristic and returns zero.
     * @param data Unused source data.
     * @param size Unused data size.
     * @return Always zero.
     */
    std::size_t Write(const std::uint8_t* data, std::size_t size) override;

private:
    SimpleBLE::Peripheral peripheral_;
};

} // namespace cpptoolkit::net
