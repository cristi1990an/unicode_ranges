cmake_minimum_required(VERSION 3.24)

project(unicode_ranges VERSION 0.2.1 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Threads REQUIRED)

file(GLOB UTF8_RANGES_PUBLIC_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unicode_ranges/*.hpp"
)

option(UTF8_RANGES_BUILD_TESTS "Build the unicode_ranges test runner" ${PROJECT_IS_TOP_LEVEL})
set(UTF8_RANGES_CONFORMANCE_DATA "" CACHE PATH "Optional matching-version UCD directory containing normalization and grapheme tests")
option(UTF8_RANGES_BUILD_BENCHMARKS "Build the unicode_ranges benchmark runner" ${PROJECT_IS_TOP_LEVEL})
option(UTF8_RANGES_ENABLE_ICU "Enable ICU-backed locale-aware casing when ICU is available" ON)
option(UTF8_RANGES_FETCH_SIMDUTF "Fetch simdutf when no package config is available" ${PROJECT_IS_TOP_LEVEL})
option(UTF8_RANGES_ENABLE_TEST_HOOKS "Enable internal test hooks used by selected CI lanes" OFF)
option(UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL "Force UTF-32 parallel test paths when test hooks are enabled" OFF)

if(UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL)
    set(UTF8_RANGES_ENABLE_TEST_HOOKS ON)
endif()

set(UTF8_RANGES_WITH_ICU OFF)
if(UTF8_RANGES_ENABLE_ICU)
    find_package(ICU QUIET COMPONENTS uc i18n)
    if(ICU_FOUND)
        set(UTF8_RANGES_WITH_ICU ON)
    endif()
endif()

find_package(simdutf CONFIG QUIET)
if(NOT simdutf_FOUND AND NOT TARGET simdutf::simdutf)
    if(NOT UTF8_RANGES_FETCH_SIMDUTF)
        message(FATAL_ERROR
            "simdutf was not found. Install simdutf or enable UTF8_RANGES_FETCH_SIMDUTF.")
    endif()

    include(FetchContent)
    set(SIMDUTF_TESTS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_TOOLS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        simdutf
        GIT_REPOSITORY https://github.com/simdutf/simdutf.git
        GIT_TAG v8.2.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(simdutf)
endif()

if(UTF8_RANGES_WITH_ICU)
    set(UTF8_RANGES_CONFIG_HAS_ICU 1)
else()
    set(UTF8_RANGES_CONFIG_HAS_ICU 0)
endif()
set(UTF8_RANGES_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${UTF8_RANGES_GENERATED_INCLUDE_DIR}/unicode_ranges")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/unicode_ranges_config.hpp.in"
    "${UTF8_RANGES_GENERATED_INCLUDE_DIR}/unicode_ranges/config.hpp"
    @ONLY
)

add_library(unicode_ranges STATIC
    unicode_ranges.cpp
    unicode_tables_runtime.cpp
)
add_library(unicode_ranges::unicode_ranges ALIAS unicode_ranges)

target_compile_features(unicode_ranges PUBLIC cxx_std_23)
target_include_directories(unicode_ranges
    PUBLIC
        $<BUILD_INTERFACE:${UTF8_RANGES_GENERATED_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(unicode_ranges PUBLIC Threads::Threads)
target_link_libraries(unicode_ranges
    PUBLIC
        $<BUILD_INTERFACE:simdutf::simdutf>
        $<INSTALL_INTERFACE:simdutf::simdutf>
)

if(MSVC)
    target_compile_options(unicode_ranges PUBLIC /utf-8)
    target_compile_options(unicode_ranges PRIVATE /bigobj)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(unicode_ranges PRIVATE -fvisibility=hidden)
endif()

if(UTF8_RANGES_WITH_ICU)
    target_link_libraries(unicode_ranges PUBLIC ICU::uc ICU::i18n)
endif()

if(UTF8_RANGES_ENABLE_TEST_HOOKS)
    target_compile_definitions(unicode_ranges PUBLIC UTF8_RANGES_ENABLE_TEST_HOOKS=1)
endif()

if(UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL)
    target_compile_definitions(unicode_ranges PUBLIC UTF8_RANGES_TEST_FORCE_UTF32_PARALLEL=1)
endif()

if(UTF8_RANGES_BUILD_TESTS)
    enable_testing()
    add_executable(unicode_ranges_tests source.cpp)
    target_compile_features(unicode_ranges_tests PRIVATE cxx_std_23)
    if(MSVC)
        target_compile_options(unicode_ranges_tests PRIVATE /utf-8 /bigobj)
    endif()
    target_link_libraries(unicode_ranges_tests PRIVATE unicode_ranges::unicode_ranges)
    add_test(NAME unicode_ranges_tests COMMAND unicode_ranges_tests)
    if(UTF8_RANGES_CONFORMANCE_DATA)
        add_test(NAME unicode_ranges_conformance
            COMMAND unicode_ranges_tests --conformance-dir "${UTF8_RANGES_CONFORMANCE_DATA}")
    endif()
endif()

if(UTF8_RANGES_BUILD_BENCHMARKS)
    add_executable(unicode_ranges_benchmarks tools/benchmarks/unicode_ranges_benchmarks.cpp)
    target_compile_features(unicode_ranges_benchmarks PRIVATE cxx_std_23)
    if(MSVC)
        target_compile_options(unicode_ranges_benchmarks PRIVATE /utf-8 /bigobj)
    endif()
    target_link_libraries(unicode_ranges_benchmarks PRIVATE unicode_ranges::unicode_ranges)
endif()

install(TARGETS unicode_ranges
    EXPORT unicode_rangesTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    unicode_ranges_borrowed.hpp
    unicode_ranges_all.hpp
    unicode_ranges.hpp
    unicode_ranges_full.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    "${UTF8_RANGES_GENERATED_INCLUDE_DIR}/unicode_ranges/config.hpp"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/unicode_ranges"
)

foreach(header IN LISTS UTF8_RANGES_PUBLIC_HEADERS)
    get_filename_component(header_directory "${header}" DIRECTORY)
    file(RELATIVE_PATH header_subdir "${CMAKE_CURRENT_SOURCE_DIR}" "${header_directory}")
    install(FILES "${header}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${header_subdir}")
endforeach()

install(FILES
    LICENSE
    LICENSE-MIT
    LICENSE-APACHE
    LICENSE-UNICODE
    THIRD_PARTY_NOTICES.md
    README.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/unicode_ranges
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_rangesConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

set(UTF8_RANGES_PACKAGE_HAS_ICU ${UTF8_RANGES_WITH_ICU})
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/unicode_rangesConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_rangesConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/unicode_ranges"
)

install(EXPORT unicode_rangesTargets
    FILE unicode_rangesTargets.cmake
    NAMESPACE unicode_ranges::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unicode_ranges
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_rangesConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_rangesConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unicode_ranges
)

get_target_property(UTF8_RANGES_SIMDUTF_TARGET_ALIAS simdutf::simdutf ALIASED_TARGET)
if(UTF8_RANGES_SIMDUTF_TARGET_ALIAS)
    set(UTF8_RANGES_SIMDUTF_TARGET "${UTF8_RANGES_SIMDUTF_TARGET_ALIAS}")
else()
    set(UTF8_RANGES_SIMDUTF_TARGET simdutf::simdutf)
endif()
get_target_property(UTF8_RANGES_SIMDUTF_TARGET_IMPORTED "${UTF8_RANGES_SIMDUTF_TARGET}" IMPORTED)
if(UTF8_RANGES_SIMDUTF_TARGET_IMPORTED)
    export(EXPORT unicode_rangesTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/unicode_rangesTargets.cmake"
        NAMESPACE unicode_ranges::
    )
endif()
