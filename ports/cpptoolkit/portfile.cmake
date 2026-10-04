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

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DCPPTOOLKIT_BUILD_TESTS=OFF
        -DCPPTOOLKIT_BUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME cpptoolkit CONFIG_PATH lib/cmake/cpptoolkit)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/cpptoolkit" RENAME copyright)
