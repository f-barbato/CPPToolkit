cmake_minimum_required(VERSION 3.20)

project(CPPToolkit
    VERSION 0.1.0
    DESCRIPTION "Modular C++ toolkit: platform, struct, mvvm, algo, ui, net"
    HOMEPAGE_URL "https://github.com/f-barbato/CPPToolkit"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CPPToolkitModule)

# ---------------------------------------------------------------------------
# Per-module build options (one per vcpkg feature)
# ---------------------------------------------------------------------------
option(CPPTOOLKIT_BUILD_PLATFORM "Build the platform module" OFF)
option(CPPTOOLKIT_BUILD_STRUCT "Build the struct module" OFF)
option(CPPTOOLKIT_BUILD_MVVM "Build the mvvm module" OFF)
option(CPPTOOLKIT_BUILD_ALGO "Build the algo module" OFF)
option(CPPTOOLKIT_BUILD_UI "Build the ui module" OFF)
option(CPPTOOLKIT_BUILD_NET "Build the net module" OFF)
option(CPPTOOLKIT_BUILD_BLE "Enable the BLE transport in the net module (requires simpleble)" OFF)
option(CPPTOOLKIT_BUILD_UI_NET_WIDGETS "Enable ui widgets that visualize net module state (requires ui + net)" OFF)

option(CPPTOOLKIT_BUILD_TESTS "Build unit tests" OFF)
option(CPPTOOLKIT_BUILD_EXAMPLES "Build examples" OFF)

# ---------------------------------------------------------------------------
# Hard inter-module dependencies: enabling a module force-enables what it needs.
# ---------------------------------------------------------------------------
if(CPPTOOLKIT_BUILD_UI_NET_WIDGETS)
    set(CPPTOOLKIT_BUILD_UI ON CACHE BOOL "" FORCE)
    set(CPPTOOLKIT_BUILD_NET ON CACHE BOOL "" FORCE)
endif()
if(CPPTOOLKIT_BUILD_BLE)
    set(CPPTOOLKIT_BUILD_NET ON CACHE BOOL "" FORCE)
endif()
if(CPPTOOLKIT_BUILD_UI)
    set(CPPTOOLKIT_BUILD_MVVM ON CACHE BOOL "" FORCE)
endif()
if(CPPTOOLKIT_BUILD_MVVM)
    set(CPPTOOLKIT_BUILD_STRUCT ON CACHE BOOL "" FORCE)
endif()

if(NOT (CPPTOOLKIT_BUILD_PLATFORM OR CPPTOOLKIT_BUILD_STRUCT OR CPPTOOLKIT_BUILD_MVVM OR
        CPPTOOLKIT_BUILD_ALGO OR CPPTOOLKIT_BUILD_UI OR CPPTOOLKIT_BUILD_NET))
    message(WARNING "CPPToolkit: no module enabled. Enable at least one CPPTOOLKIT_BUILD_<MODULE> option.")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# cpptoolkitTargets is populated by cpptoolkit_add_module() as each module is processed.
# Order matters: 'net' must be configured before 'ui' so that ui can detect
# the cpptoolkit::net target for its optional net-widgets sub-functionality.
foreach(module platform struct net mvvm algo ui)
    string(TOUPPER "${module}" MODULE_UPPER)
    if(CPPTOOLKIT_BUILD_${MODULE_UPPER})
        add_subdirectory(modules/${module})
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Package config for find_package(cpptoolkit CONFIG REQUIRED)
# ---------------------------------------------------------------------------
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPPToolkitConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cpptoolkit-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpptoolkit"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cpptoolkit-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cpptoolkit-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cpptoolkit-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpptoolkit"
)
install(
    EXPORT cpptoolkitTargets
    FILE cpptoolkitTargets.cmake
    NAMESPACE cpptoolkit::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpptoolkit"
)
