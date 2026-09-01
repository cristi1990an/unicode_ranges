cmake_minimum_required(VERSION 3.24)

project(unicode_ranges_conan_test_package LANGUAGES CXX)

find_package(unicode_ranges CONFIG REQUIRED)

add_executable(unicode_ranges_conan_test_package src/main.cpp)
target_compile_features(unicode_ranges_conan_test_package PRIVATE cxx_std_23)
target_link_libraries(
    unicode_ranges_conan_test_package
    PRIVATE unicode_ranges::unicode_ranges
)

if(MSVC)
    target_compile_options(unicode_ranges_conan_test_package PRIVATE /utf-8 /bigobj)
endif()
