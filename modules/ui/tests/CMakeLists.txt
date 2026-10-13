find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(cpptoolkit_ui_tests TextBoxWidgetTests.cpp WidgetGalleryTests.cpp InputWidgetsTests.cpp RaylibRenderingTests.cpp StylerTests.cpp FilePathWidgetTests.cpp ShortcutTests.cpp
    ../examples/TestUI.cpp ../examples/TestUIState.cpp)
target_link_libraries(cpptoolkit_ui_tests PRIVATE cpptoolkit::ui GTest::gtest_main)
if(TARGET cpptoolkit::concurrency)
    target_sources(cpptoolkit_ui_tests PRIVATE ../examples/CancellationExample.cpp)
    target_link_libraries(cpptoolkit_ui_tests PRIVATE cpptoolkit::concurrency)
    target_compile_definitions(cpptoolkit_ui_tests PRIVATE CPPTOOLKIT_TESTUI_HAS_CONCURRENCY=1)
endif()
gtest_discover_tests(cpptoolkit_ui_tests)
