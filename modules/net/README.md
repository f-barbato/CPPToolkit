# net

Networking / communication transports behind a common abstraction.

## API

- `ITransport` — common interface (`Connect()`, `Disconnect()`, `IsConnected()`, `Read()`, `Write()`) implemented by every transport below.
- `SerialTransport` — UART/USB-CDC transport (stub; platform-specific I/O is TODO).
- `TcpTransport` — TCP/IP transport (stub; platform-specific I/O is TODO).
- `BleTransport` — Bluetooth Low Energy transport backed by [SimpleBLE](https://github.com/simpleble/simpleble). Only built when `CPPTOOLKIT_BUILD_BLE` (vcpkg feature `ble`) is enabled.

## Dependencies

- Base module: none.
- `ble` sub-feature: `simpleble` (⚠️ licensed under **BUSL-1.1**, not OSI-permissive — verify terms before commercial distribution).

## Usage

```cpp
#include <cpptoolkit/net/net.h>

cpptoolkit::net::SerialTransport serial("/dev/ttyACM0", 115200);
if (serial.Connect()) {
    std::uint8_t buf[64];
    serial.Read(buf, sizeof(buf));
}
```
