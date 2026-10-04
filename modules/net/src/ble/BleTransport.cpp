#include "cpptoolkit/net/ble/BleTransport.h"

namespace cpptoolkit::net {

BleTransport::BleTransport(SimpleBLE::Peripheral peripheral) : peripheral_(std::move(peripheral)) {}

bool BleTransport::Connect() {
    if (!peripheral_.initialized()) return false;
    peripheral_.connect();
    return peripheral_.is_connected();
}

void BleTransport::Disconnect() {
    if (peripheral_.initialized() && peripheral_.is_connected()) {
        peripheral_.disconnect();
    }
}

bool BleTransport::IsConnected() const { return peripheral_.initialized() && const_cast<SimpleBLE::Peripheral&>(peripheral_).is_connected(); }

std::size_t BleTransport::Read(std::uint8_t* /*buffer*/, std::size_t /*maxSize*/) {
    // TODO: read from the selected GATT characteristic.
    return 0;
}

std::size_t BleTransport::Write(const std::uint8_t* /*data*/, std::size_t /*size*/) {
    // TODO: write to the selected GATT characteristic.
    return 0;
}

} // namespace cpptoolkit::net
