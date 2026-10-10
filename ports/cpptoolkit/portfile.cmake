# Overlay port for CPPToolkit.
#
# NOTE: keep this vcpkg.json in sync with the repository root's vcpkg.json —
# the root manifest is used for local/standalone builds of this repo, while
# this one is what downstream consumers get when they add "cpptoolkit" as a
# dependency (via this overlay port, until/unless the package is published
# to the public vcpkg registry).

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        platform        CPPTOOLKIT_BUILD_PLATFORM
        struct          CPPTOOLKIT_BUILD_STRUCT
        mvvm            CPPTOOLKIT_BUILD_MVVM
        algo            CPPTOOLKIT_BUILD_ALGO
        ui              CPPTOOLKIT_BUILD_UI
        net             CPPTOOLKIT_BUILD_NET
        ble             CPPTOOLKIT_BUILD_BLE
        ui-net-widgets  CPPTOOLKIT_BUILD_UI_NET_WIDGETS
)

# TODO: once this repository is published, switch to vcpkg_from_github() with
# a real tag and let vcpkg report the correct SHA512 on first build failure:
#
# vcpkg_from_github(
#     OUT_SOURCE_PATH SOURCE_PATH
#     REPO f-barbato/CPPToolkit
#     REF "v${VERSION}"
#     SHA512 0
#     HEAD_REF main
# )
#
# Until then, this overlay port builds directly from the repository checked
# out on disk (this port is expected to live at <repo>/ports/cpptoolkit).
get_filename_component(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BRIDGE_OPTIONS "")
if("ui" IN_LIST FEATURES OR "ui-net-widgets" IN_LIST FEATURES)
    vcpkg_from_github(
        OUT_SOURCE_PATH RLIMGUI_SOURCE_PATH
        REPO raylib-extras/rlImGui
        REF 1550009359ad975927f7f0e4a3f47e3f27123ea9
        SHA512 5c3fb4b756d687b44fe5fc241256232e0a22528a34832d93351dcd282314ad866d2fa242eef1ed08ad5111e89e51e0614d6c19159e3cda3a2ae76ca3591912b9
    )
    list(APPEND BRIDGE_OPTIONS "-DFETCHCONTENT_SOURCE_DIR_RLIMGUI=${RLIMGUI_SOURCE_PATH}")
endif()

if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
    set(CPPTOOLKIT_SHARED ON)
else()
    set(CPPTOOLKIT_SHARED OFF)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        ${BRIDGE_OPTIONS}
        -DBUILD_SHARED_LIBS=${CPPTOOLKIT_SHARED}
        -DCPPTOOLKIT_BUILD_TESTS=OFF
        -DCPPTOOLKIT_BUILD_EXAMPLES=OFF
        -DCPPTOOLKIT_BUILD_DOCS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME cpptoolkit CONFIG_PATH lib/cmake/cpptoolkit)
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/cpptoolkit" RENAME copyright)
