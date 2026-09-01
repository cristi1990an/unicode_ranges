cmake_minimum_required(VERSION 3.24)

project(unicode_ranges_vcpkg_test_project LANGUAGES CXX)

find_package(unicode_ranges CONFIG REQUIRED)

add_executable(
    unicode_ranges_vcpkg_test_project
    ../../../tools/package_smoke_consumer/main.cpp
)
target_compile_features(unicode_ranges_vcpkg_test_project PRIVATE cxx_std_23)
target_link_libraries(
    unicode_ranges_vcpkg_test_project
    PRIVATE unicode_ranges::unicode_ranges
)

if(MSVC)
    target_compile_options(unicode_ranges_vcpkg_test_project PRIVATE /utf-8 /bigobj)
endif()

enable_testing()
add_test(
    NAME unicode_ranges_vcpkg_test_project
    COMMAND unicode_ranges_vcpkg_test_project
)
