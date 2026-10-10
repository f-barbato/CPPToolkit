#pragma once

/**
 * @file net.h
 * @brief Umbrella header for the CPPToolkit networking module.
 *
 * Includes ITransport and the serial and TCP transport declarations.
 * BleTransport is included when CPPTOOLKIT_NET_HAS_BLE is defined.
 */

#include "cpptoolkit/net/ITransport.h"
#include "cpptoolkit/net/serial/SerialTransport.h"
#include "cpptoolkit/net/tcp/TcpTransport.h"

#if defined(CPPTOOLKIT_NET_HAS_BLE)
#include "cpptoolkit/net/ble/BleTransport.h"
#endif
