#pragma once

/**
 * @file Platform.h
 * @brief Platform identification utilities.
 */

namespace cpptoolkit::platform {

/** Operating-system categories recognized by CPPToolkit. */
enum class OperatingSystem {
    /** Microsoft Windows. */
    Windows,
    /** Linux. */
    Linux,
    /** macOS. */
    MacOS,
    /** Any platform not recognized by the compile-time checks. */
    Unknown
};

/**
 * @brief Identifies the operating system using compile-time platform macros.
 * @return The operating system selected by the compiler's predefined macros,
 * or OperatingSystem::Unknown when no supported macro is defined.
 */
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

/**
 * @brief Returns a static English name for an operating-system value.
 * @param os The value to convert.
 * @return "Windows", "macOS", "Linux", or "Unknown" for unrecognized values.
 */
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
