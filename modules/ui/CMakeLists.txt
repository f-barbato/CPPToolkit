find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)

cpptoolkit_add_module(
    NAME ui
    PUBLIC_DEPS
        cpptoolkit::mvvm
        imgui::imgui
        implot::implot
)

# Optional sub-functionality: widgets that visualize cpptoolkit::net state.
# 'net' is configured before 'ui' in the root CMakeLists.txt, so the target
# already exists here if CPPTOOLKIT_BUILD_NET is ON. This mirrors the
# "ui-net-widgets" vcpkg feature, which forces both CPPTOOLKIT_BUILD_UI and
# CPPTOOLKIT_BUILD_NET on.
if(TARGET cpptoolkit::net)
    target_compile_definitions(cpptoolkit_ui INTERFACE CPPTOOLKIT_UI_HAS_NET_WIDGETS=1)
    target_link_libraries(cpptoolkit_ui INTERFACE cpptoolkit::net)
endif()
