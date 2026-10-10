include(FetchContent)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
# rlImGui (the raylib <-> Dear ImGui bridge) is not available on vcpkg — it is
# just two loose source files upstream, so it is vendored here via
# FetchContent and built as a small static library, rather than inside the
# cpptoolkit::ui library itself (ui only depends on imgui/implot directly).
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
)
FetchContent_MakeAvailable(rlimgui)

add_library(rlImGui STATIC "${rlimgui_SOURCE_DIR}/rlImGui.cpp")
target_include_directories(rlImGui PUBLIC
    $<BUILD_INTERFACE:${rlimgui_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(rlImGui PUBLIC raylib imgui::imgui)

# rlImGui is a public dependency of cpptoolkit::ui (exposed in
# Application.h), so it must be installed/exported alongside it rather than
# left dangling as a build-only FetchContent target, otherwise
# `install(EXPORT cpptoolkitTargets ...)` fails because cpptoolkit_ui
# requires a target that isn't part of any export set.
install(TARGETS rlImGui EXPORT cpptoolkitTargets)
install(FILES "${rlimgui_SOURCE_DIR}/rlImGui.h" "${rlimgui_SOURCE_DIR}/imgui_impl_raylib.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(UI_SOURCES
    src/widgets/ButtonWidget.cpp
    src/widgets/PlotLineWidget.cpp
    src/widgets/SliderFloatWidget.cpp
    src/widgets/TextBoxWidget.cpp
    src/widgets/FeedbackWidgets.cpp
    src/widgets/DataWidgets.cpp
    src/widgets/LayoutWidgets.cpp
    src/widgets/NavigationWidgets.cpp
    src/widgets/OverlayWidgets.cpp
    src/widgets/InputWidgets.cpp
    src/widgets/TextWidget.cpp
    src/Application.cpp
)

cpptoolkit_add_module(
    NAME ui
    SOURCES ${UI_SOURCES}
    PUBLIC_DEPS
        rlImGui
        cpptoolkit::mvvm
        imgui::imgui
        implot::implot
        glfw
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
