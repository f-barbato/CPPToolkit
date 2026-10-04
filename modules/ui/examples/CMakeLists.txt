include(FetchContent)

find_package(raylib CONFIG REQUIRED)

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

find_package(imgui CONFIG REQUIRED)

add_library(rlImGui STATIC "${rlimgui_SOURCE_DIR}/rlImGui.cpp")
target_include_directories(rlImGui PUBLIC "${rlimgui_SOURCE_DIR}")
target_link_libraries(rlImGui PUBLIC raylib imgui::imgui)

add_executable(cpptoolkit_ui_demo main.cpp)
target_link_libraries(cpptoolkit_ui_demo PRIVATE cpptoolkit::ui rlImGui)
target_compile_features(cpptoolkit_ui_demo PRIVATE cxx_std_17)
