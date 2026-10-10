

find_package(raylib CONFIG REQUIRED)



add_executable(cpptoolkit_ui_demo main.cpp)
target_link_libraries(cpptoolkit_ui_demo PRIVATE cpptoolkit::ui rlImGui)
target_compile_features(cpptoolkit_ui_demo PRIVATE cxx_std_17)

# Serial console panel example: optional, only if the net module is enabled.
if(TARGET cpptoolkit::net)
    target_link_libraries(cpptoolkit_ui_demo PRIVATE cpptoolkit::net)
    target_compile_definitions(cpptoolkit_ui_demo PRIVATE CPPTOOLKIT_UI_DEMO_HAS_NET=1)
endif()
