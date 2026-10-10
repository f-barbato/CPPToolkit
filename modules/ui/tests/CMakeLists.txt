find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(cpptoolkit_ui_tests TextBoxWidgetTests.cpp WidgetGalleryTests.cpp InputWidgetsTests.cpp)
target_link_libraries(cpptoolkit_ui_tests PRIVATE cpptoolkit::ui GTest::gtest_main)
gtest_discover_tests(cpptoolkit_ui_tests)
