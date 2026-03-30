# Install And Integrate

`unicode_ranges` is a header-only library. Today, the supported consumption model is "bring the sources into your tree and add the repository root as an include directory."

## Current packaging status

- There is no first-party package-manager distribution yet.
- There is no first-party CMake package or `install()` export yet.
- There may be no tagged release that matches the commit you want to consume.

So the practical choices right now are:

1. Vendor a snapshot of the repository into your source tree.
2. Add the repository as a git submodule.
3. Fetch the repository in CMake, then expose it as an interface include path yourself.

## What your build needs

- C++23 enabled
- the repository root on the include path
- `#include "unicode_ranges.hpp"` in user code

The public umbrella header lives at the repository root:

```cpp
#include "unicode_ranges.hpp"
```

## Recommended: vendor or submodule

If you vendor the repository or add it as a submodule, point your include path at the checked-out root:

```text
your_project/
  third_party/
    unicode_ranges/
      unicode_ranges.hpp
      unicode_ranges/
```

Then compile with:

- include directory: `third_party/unicode_ranges`
- language mode: C++23

## CMake: manual interface target

Because the repository does not yet ship a first-party CMake package, the simplest CMake integration is an interface target:

```cmake
add_library(unicode_ranges INTERFACE)
target_include_directories(unicode_ranges INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicode_ranges
)
target_compile_features(unicode_ranges INTERFACE cxx_std_23)
```

Then consume it normally:

```cmake
target_link_libraries(your_target PRIVATE unicode_ranges)
```

## CMake: FetchContent

If you prefer to fetch sources at configure time, keep the same interface-target pattern:

```cmake
include(FetchContent)

FetchContent_Declare(
    unicode_ranges_src
    GIT_REPOSITORY https://github.com/cristi1990an/unicode_ranges.git
    GIT_TAG main
)

FetchContent_MakeAvailable(unicode_ranges_src)

add_library(unicode_ranges INTERFACE)
target_include_directories(unicode_ranges INTERFACE
    ${unicode_ranges_src_SOURCE_DIR}
)
target_compile_features(unicode_ranges INTERFACE cxx_std_23)
```

This is a source-fetch recipe, not a first-party packaged install.

## Toolchains exercised in CI

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

## Next steps

- [Getting Started](getting-started.md)
- [Common Tasks](common-tasks.md)
- [Design](design.md)
