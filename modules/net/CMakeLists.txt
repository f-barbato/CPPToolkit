set(NET_SOURCES
    src/serial/SerialTransport.cpp
    src/tcp/TcpTransport.cpp
)
set(NET_PUBLIC_DEPS "")
set(NET_DEFINITIONS "")

if(CPPTOOLKIT_BUILD_BLE)
    find_package(simpleble CONFIG REQUIRED)
    list(APPEND NET_SOURCES src/ble/BleTransport.cpp)
    list(APPEND NET_PUBLIC_DEPS simpleble::simpleble)
    list(APPEND NET_DEFINITIONS CPPTOOLKIT_NET_HAS_BLE=1)
endif()

cpptoolkit_add_module(
    NAME net
    SOURCES ${NET_SOURCES}
    PUBLIC_DEPS ${NET_PUBLIC_DEPS}
    DEFINITIONS ${NET_DEFINITIONS}
)
