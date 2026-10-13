find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(cpptoolkit_mvvm_tests UndoStackTests.cpp)
target_link_libraries(cpptoolkit_mvvm_tests PRIVATE cpptoolkit::mvvm GTest::gtest_main)
gtest_discover_tests(cpptoolkit_mvvm_tests)
