#pragma once

/**
 * @file Platform.h
 * @brief Platform identification utilities.
 */

/**
 * @def CPPTOOLKIT_OS_WINDOWS
 * @brief 1 when compiling for Windows, otherwise 0. Usable in `#if`.
 * @def CPPTOOLKIT_OS_MACOS
 * @brief 1 when compiling for macOS, otherwise 0.
 * @def CPPTOOLKIT_OS_LINUX
 * @brief 1 when compiling for Linux, otherwise 0.
 * @def CPPTOOLKIT_OS_UNKNOWN
 * @brief 1 when no supported operating system is detected, otherwise 0.
 *
 * Exactly one of these macros is 1. They are evaluated at preprocessing time,
 * so they can guard includes and platform-specific declarations.
 */
#if defined(_WIN32)
#define CPPTOOLKIT_OS_WINDOWS 1
#define CPPTOOLKIT_OS_MACOS 0
#define CPPTOOLKIT_OS_LINUX 0
#define CPPTOOLKIT_OS_UNKNOWN 0
#elif defined(__APPLE__)
#define CPPTOOLKIT_OS_WINDOWS 0
#define CPPTOOLKIT_OS_MACOS 1
#define CPPTOOLKIT_OS_LINUX 0
#define CPPTOOLKIT_OS_UNKNOWN 0
#elif defined(__linux__)
#define CPPTOOLKIT_OS_WINDOWS 0
#define CPPTOOLKIT_OS_MACOS 0
#define CPPTOOLKIT_OS_LINUX 1
#define CPPTOOLKIT_OS_UNKNOWN 0
#else
#define CPPTOOLKIT_OS_WINDOWS 0
#define CPPTOOLKIT_OS_MACOS 0
#define CPPTOOLKIT_OS_LINUX 0
#define CPPTOOLKIT_OS_UNKNOWN 1
#endif

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
#if CPPTOOLKIT_OS_WINDOWS
    return OperatingSystem::Windows;
#elif CPPTOOLKIT_OS_MACOS
    return OperatingSystem::MacOS;
#elif CPPTOOLKIT_OS_LINUX
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
