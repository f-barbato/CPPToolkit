#pragma once

namespace cpptoolkit::platform {

enum class OperatingSystem {
    Windows,
    Linux,
    MacOS,
    Unknown
};

constexpr OperatingSystem GetCurrentOS() {
#if defined(_WIN32)
    return OperatingSystem::Windows;
#elif defined(__APPLE__)
    return OperatingSystem::MacOS;
#elif defined(__linux__)
    return OperatingSystem::Linux;
#else
    return OperatingSystem::Unknown;
#endif
}

constexpr const char* ToString(OperatingSystem os) {
    switch (os) {
        case OperatingSystem::Windows:
            return "Windows";
        case OperatingSystem::MacOS:
            return "macOS";
        case OperatingSystem::Linux:
            return "Linux";
        default:
            return "Unknown";
    }
}

} // namespace cpptoolkit::platform
