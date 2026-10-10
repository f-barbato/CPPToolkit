cmake_minimum_required(VERSION 3.21)
project(CPPToolkitPackageConsumer LANGUAGES CXX)

option(CPPTOOLKIT_CONSUMER_UI "Exercise the installed UI package" OFF)
find_package(cpptoolkit CONFIG REQUIRED)
enable_testing()

add_executable(package_consumer main.cpp)
target_link_libraries(package_consumer PRIVATE
    cpptoolkit::platform cpptoolkit::struct cpptoolkit::mvvm cpptoolkit::net cpptoolkit::algo
)
if(CPPTOOLKIT_CONSUMER_UI)
    target_link_libraries(package_consumer PRIVATE cpptoolkit::ui)
    target_compile_definitions(package_consumer PRIVATE CPPTOOLKIT_CONSUMER_UI=1)
endif()
add_test(NAME installed_package_consumer COMMAND package_consumer)

if(WIN32)
    add_custom_command(TARGET package_consumer POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            $<TARGET_RUNTIME_DLLS:package_consumer> $<TARGET_FILE_DIR:package_consumer>
        COMMAND_EXPAND_LISTS
    )
endif()
