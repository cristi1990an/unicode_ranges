# Install And Integrate

`unicode_ranges` is now a compiled library with a header-first public API. The supported integration model is:

1. bring this repository into your tree
2. build and link the `unicode_ranges` library target, or an equivalent library target in your own build
3. place a pinned `simdutf` singleheader release on the include path

## Current packaging status

- There is no first-party package-manager distribution yet.
- There is no first-party CMake package or `install()` export yet.
- There may be no tagged release that matches the commit you want to consume.
- Runtime UTF validation and UTF-8 <-> UTF-16/UTF-32 transcoding require pinned `simdutf` `v7.7.0`.
- The repository itself now ships a first-party Visual Studio static-library project, `unicode_ranges.vcxproj`, plus separate test and benchmark executables that link it.

So the practical choices right now are:

1. Vendor a snapshot of the repository into your source tree.
2. Add the repository as a git submodule.
3. Fetch the repository in CMake, then build a small static library target yourself.

## What your build needs

- C++23 enabled
- the repository root on the include path
- a `simdutf` singleheader release root on the include path
- `unicode_ranges.cpp` compiled into your `unicode_ranges` library target
- `#include "unicode_ranges.hpp"` in user code

The public umbrella header lives at the repository root:

```cpp
#include "unicode_ranges.hpp"
```

## Recommended: vendor or submodule

If you vendor the repository or add it as a submodule, point your include path at the checked-out repository root and at a pinned `simdutf` singleheader release:

```text
your_project/
  third_party/
    unicode_ranges/
      unicode_ranges.cpp
      unicode_ranges.hpp
      unicode_ranges/
    simdutf/
      simdutf.h
      simdutf.cpp
```

Then build with:

- include directories:
  - `third_party/unicode_ranges`
  - `third_party/simdutf`
- language mode: C++23
- one compiled library target:
  - compile `third_party/unicode_ranges/unicode_ranges.cpp` into `unicode_ranges`
  - link your executable or test target against that library

## Visual Studio

The repository now contains a first-party Visual Studio library project:

- `unicode_ranges.vcxproj`: static library
- `unicode_ranges_tests.vcxproj`: test runner linked against the library
- `unicode_ranges_benchmarks.vcxproj`: benchmark runner linked against the library
- `comparative_benchmarks.vcxproj`: comparative benchmark runner linked against the library

If you are consuming the repository directly from Visual Studio, build `unicode_ranges.vcxproj` and link it into your own executable or test project the same way the repo's test/benchmark projects do.

## CMake: manual target

Because the repository does not yet ship a first-party CMake package, the simplest CMake integration is still a small static library target:

```cmake
add_library(unicode_ranges STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicode_ranges/unicode_ranges.cpp
)
target_include_directories(unicode_ranges PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicode_ranges
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdutf
)
target_compile_features(unicode_ranges PUBLIC cxx_std_23)
```

Then consume it normally:

```cmake
target_link_libraries(your_target PRIVATE unicode_ranges)
```

## Optional ICU-backed locale casing

The default library build depends only on pinned `simdutf` and exposes only locale-independent Unicode casing.

If you want ICU-backed locale-sensitive casing overloads such as `to_lowercase("tr"_locale)`, `to_uppercase("tr"_locale)`, or `case_fold("tr"_locale)`, enable ICU on the `unicode_ranges` target itself:

- find and link ICU in the library target
- define `UTF8_RANGES_ENABLE_ICU=1` on the library target
- let the extra overloads appear only in that configuration

Example:

```cmake
option(UNICODE_RANGES_WITH_ICU "Enable ICU-backed locale casing overloads" OFF)

add_library(unicode_ranges STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicode_ranges/unicode_ranges.cpp
)
target_include_directories(unicode_ranges PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicode_ranges
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdutf
)
target_compile_features(unicode_ranges PUBLIC cxx_std_23)

if(UNICODE_RANGES_WITH_ICU)
    find_package(ICU REQUIRED COMPONENTS uc)
    target_compile_definitions(unicode_ranges PUBLIC UTF8_RANGES_ENABLE_ICU=1)
    target_link_libraries(unicode_ranges PUBLIC ICU::uc)
endif()
```

This keeps the default build small while making the locale-sensitive overloads disappear entirely when ICU is not available.

When ICU is enabled, locale-aware casing follows ICU locale resolution behavior. `locale_id` is a raw null-terminated locale-name token, while `_locale` gives you a compile-time checked literal form. The locale-aware overloads pass the token through to ICU, which may canonicalize it or fall back to a more general locale instead of failing. Use `is_available_locale(...)` if you want to require that the current ICU data set explicitly exposes a locale before calling a locale-aware casing overload.

## CMake: FetchContent

If you prefer to fetch sources at configure time, fetch both repositories and keep the same compiled-library pattern:

```cmake
include(FetchContent)

FetchContent_Declare(
    unicode_ranges_src
    GIT_REPOSITORY https://github.com/cristi1990an/unicode_ranges.git
    GIT_TAG main
)

FetchContent_Declare(
    simdutf_src
    URL https://github.com/simdutf/simdutf/releases/download/v7.7.0/singleheader.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(unicode_ranges_src)
FetchContent_MakeAvailable(simdutf_src)

add_library(unicode_ranges STATIC
    ${unicode_ranges_src_SOURCE_DIR}/unicode_ranges.cpp
)
target_include_directories(unicode_ranges PUBLIC
    ${unicode_ranges_src_SOURCE_DIR}
    ${simdutf_src_SOURCE_DIR}
)
target_compile_features(unicode_ranges PUBLIC cxx_std_23)
```

This is still a source-fetch recipe, not a first-party packaged install.

## Toolchains exercised in CI

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

## Next steps

- [Getting Started](getting-started.md)
- [Common Tasks](common-tasks.md)
- [Design](design.md)
