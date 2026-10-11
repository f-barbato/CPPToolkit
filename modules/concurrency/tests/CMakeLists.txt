find_package(GTest CONFIG REQUIRED)
find_package(Threads REQUIRED)
include(GoogleTest)

add_executable(cpptoolkit_concurrency_tests CancellationTokenTests.cpp WorkQueueTests.cpp)
target_link_libraries(cpptoolkit_concurrency_tests PRIVATE
    cpptoolkit::concurrency GTest::gtest_main Threads::Threads
)
gtest_discover_tests(cpptoolkit_concurrency_tests)
