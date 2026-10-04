#pragma once

#include <string>

#include <simpleble/Peripheral.h>

#include "cpptoolkit/net/ITransport.h"

namespace cpptoolkit::net {

// Bluetooth Low Energy transport backed by SimpleBLE (cross-platform:
// WinRT/CoreBluetooth/BlueZ). Only compiled when CPPTOOLKIT_BUILD_BLE is ON.
//
// NOTE: SimpleBLE is licensed under BUSL-1.1 (not OSI-permissive) — verify
// license terms before commercial distribution of binaries linking it.
//
// Read()/Write() here map to GATT characteristic notify/write, which require
// selecting a service/characteristic UUID pair first (TODO: expose via
// constructor or a Select*() method once the GATT profile for the target
// device is defined).
class BleTransport : public ITransport {
public:
    explicit BleTransport(SimpleBLE::Peripheral peripheral);

    bool Connect() override;
    void Disconnect() override;
    [[nodiscard]] bool IsConnected() const override;

    std::size_t Read(std::uint8_t* buffer, std::size_t maxSize) override;
    std::size_t Write(const std::uint8_t* data, std::size_t size) override;

private:
    SimpleBLE::Peripheral peripheral_;
};

} // namespace cpptoolkit::net
