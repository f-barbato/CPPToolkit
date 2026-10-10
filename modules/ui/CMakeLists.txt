include(FetchContent)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
if(BUILD_SHARED_LIBS)
    foreach(dependency imgui::imgui implot::implot)
        get_target_property(dependency_type ${dependency} TYPE)
        if(NOT dependency_type STREQUAL "SHARED_LIBRARY")
            message(FATAL_ERROR
                "Shared UI requires shared ${dependency} to preserve one GUI context across libraries. "
                "Use the CPPToolkit ports/ overlay and a dynamic vcpkg triplet.")
        endif()
    endforeach()
endif()
# rlImGui (the raylib <-> Dear ImGui bridge) is not available on vcpkg — it is
# vendored at a fixed revision and built with the module's linkage.
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG 1550009359ad975927f7f0e4a3f47e3f27123ea9
)
FetchContent_MakeAvailable(rlimgui)

set(rlimgui_build_dir "${CMAKE_CURRENT_BINARY_DIR}/rlimgui")
file(MAKE_DIRECTORY "${rlimgui_build_dir}")
configure_file("${rlimgui_SOURCE_DIR}/rlImGui.cpp" "${rlimgui_build_dir}/rlImGui.cpp" COPYONLY)
file(READ "${rlimgui_SOURCE_DIR}/rlImGui.h" rlimgui_public_header)
string(FIND "${rlimgui_public_header}" "// Function specifiers" rlimgui_exports_start)
string(FIND "${rlimgui_public_header}" "#ifndef NO_FONT_AWESOME" rlimgui_exports_end)
if(rlimgui_exports_start LESS 0 OR rlimgui_exports_end LESS rlimgui_exports_start)
    message(FATAL_ERROR "Unexpected rlImGui header export block")
endif()
string(SUBSTRING "${rlimgui_public_header}" 0 ${rlimgui_exports_start} rlimgui_header_prefix)
string(SUBSTRING "${rlimgui_public_header}" ${rlimgui_exports_end} -1 rlimgui_header_suffix)
file(WRITE "${rlimgui_build_dir}/rlImGui.h"
    "// CPPToolkit adaptation: independent export macros avoid conflicts with raylib.\n"
    "${rlimgui_header_prefix}#include \"rlimgui_export.h\"\n${rlimgui_header_suffix}")
# The backend is exported by rlImGui, not by the separate ImGui DLL.
file(READ "${rlimgui_SOURCE_DIR}/imgui_impl_raylib.h" rlimgui_backend_header)
string(REPLACE "IMGUI_IMPL_API" "RLIMGUIAPI" rlimgui_backend_header "${rlimgui_backend_header}")
file(WRITE "${rlimgui_build_dir}/imgui_impl_raylib.h"
    "// CPPToolkit adaptation: backend exports belong to rlImGui.\n#include \"rlImGui.h\"\n${rlimgui_backend_header}")
add_library(rlImGui "${rlimgui_build_dir}/rlImGui.cpp")
set_target_properties(rlImGui PROPERTIES POSITION_INDEPENDENT_CODE ON)
generate_export_header(rlImGui
    EXPORT_FILE_NAME "${rlimgui_build_dir}/rlimgui_export.h"
    EXPORT_MACRO_NAME RLIMGUIAPI
)
target_include_directories(rlImGui PUBLIC
    $<BUILD_INTERFACE:${rlimgui_build_dir}>
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
install(FILES "${rlimgui_build_dir}/rlImGui.h" "${rlimgui_build_dir}/imgui_impl_raylib.h"
    "${rlimgui_build_dir}/rlimgui_export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES "${rlimgui_SOURCE_DIR}/extras/IconsFontAwesome6.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/extras"
)
install(FILES "${rlimgui_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cpptoolkit/rlimgui"
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
    src/widgets/DateTimeWidgets.cpp
    src/widgets/RenderTextureWidget.cpp
    src/widgets/EditorLayoutWidget.cpp
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
# Only the explicit combined feature activates these declarations.
if(CPPTOOLKIT_BUILD_UI_NET_WIDGETS)
    target_compile_definitions(cpptoolkit_ui PUBLIC CPPTOOLKIT_UI_HAS_NET_WIDGETS=1)
    target_link_libraries(cpptoolkit_ui PUBLIC cpptoolkit::net)
endif()
