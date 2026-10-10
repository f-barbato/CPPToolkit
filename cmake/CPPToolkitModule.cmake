include_guard(GLOBAL)
include(GenerateExportHeader)

# cpptoolkit_add_module(NAME <name>
#                        [SOURCES <src...>]
#                        [PUBLIC_DEPS <target...>]
#                        [PRIVATE_DEPS <target...>]
#                        [DEFINITIONS <define...>])
#
# Declares a CPPToolkit module as a CMake target `cpptoolkit_<name>`
# (aliased as `cpptoolkit::<name>`), with consistent include directories,
# C++ standard, install/export rules, and optional tests/examples
# subdirectories. If SOURCES is omitted the module is header-only
# (INTERFACE library); compiled modules follow BUILD_SHARED_LIBS.
function(cpptoolkit_add_module)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES PUBLIC_DEPS PRIVATE_DEPS DEFINITIONS)
    cmake_parse_arguments(MOD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT MOD_NAME)
        message(FATAL_ERROR "cpptoolkit_add_module: NAME is required")
    endif()

    set(target "cpptoolkit_${MOD_NAME}")

    if(MOD_SOURCES)
        add_library(${target} ${MOD_SOURCES})
        set(scope PUBLIC)
        set_target_properties(${target} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            VERSION "${PROJECT_VERSION}"
            SOVERSION "${PROJECT_VERSION_MAJOR}"
        )
        string(TOUPPER "${MOD_NAME}" export_prefix)
        set(export_header "cpptoolkit/${MOD_NAME}/Export.h")
        generate_export_header(${target}
            EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/${export_header}"
            EXPORT_MACRO_NAME "CPPTOOLKIT_${export_prefix}_EXPORT"
            NO_EXPORT_MACRO_NAME "CPPTOOLKIT_${export_prefix}_NO_EXPORT"
            STATIC_DEFINE "CPPTOOLKIT_${export_prefix}_STATIC_DEFINE"
        )
        target_include_directories(${target} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        )
        get_target_property(module_type ${target} TYPE)
        if(module_type STREQUAL "STATIC_LIBRARY")
            target_compile_definitions(${target} PUBLIC "CPPTOOLKIT_${export_prefix}_STATIC_DEFINE")
        endif()
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/${export_header}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cpptoolkit/${MOD_NAME}"
        )
    else()
        add_library(${target} INTERFACE)
        set(scope INTERFACE)
    endif()
    add_library(cpptoolkit::${MOD_NAME} ALIAS ${target})
    # Ensure the installed/exported target is named cpptoolkit::<name> (matching
    # the in-tree ALIAS above), not cpptoolkit::cpptoolkit_<name>.
    set_target_properties(${target} PROPERTIES EXPORT_NAME ${MOD_NAME})

    target_include_directories(${target} ${scope}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_compile_features(${target} ${scope} cxx_std_23)

    if(MOD_PUBLIC_DEPS)
        target_link_libraries(${target} ${scope} ${MOD_PUBLIC_DEPS})
    endif()
    if(MOD_PRIVATE_DEPS)
        if(scope STREQUAL "INTERFACE")
            message(FATAL_ERROR
                "cpptoolkit_add_module(${MOD_NAME}): PRIVATE_DEPS requires SOURCES "
                "(a header-only/INTERFACE module cannot have PRIVATE dependencies)")
        endif()
        target_link_libraries(${target} PRIVATE ${MOD_PRIVATE_DEPS})
    endif()
    if(MOD_DEFINITIONS)
        target_compile_definitions(${target} ${scope} ${MOD_DEFINITIONS})
    endif()

    install(TARGETS ${target} EXPORT cpptoolkitTargets)
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    if(CPPTOOLKIT_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        enable_testing()
        add_subdirectory(tests)
    endif()
    if(CPPTOOLKIT_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    endif()
endfunction()
