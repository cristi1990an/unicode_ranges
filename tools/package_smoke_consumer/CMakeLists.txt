cmake_minimum_required(VERSION 3.24)

project(unicode_ranges_package_smoke LANGUAGES CXX)

find_package(unicode_ranges CONFIG REQUIRED)

add_executable(unicode_ranges_package_smoke main.cpp)
target_link_libraries(unicode_ranges_package_smoke PRIVATE unicode_ranges::unicode_ranges)
target_compile_features(unicode_ranges_package_smoke PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(unicode_ranges_package_smoke PRIVATE /utf-8 /bigobj)
endif()

enable_testing()
add_test(NAME unicode_ranges_package_smoke COMMAND unicode_ranges_package_smoke)
