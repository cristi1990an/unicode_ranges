# Install And Integrate

`unicode_ranges` is a compiled library with a header-first public API. The supported integration model is:

1. bring this repository into your tree
2. build and link the `unicode_ranges` library target, or an equivalent library target in your own build

## Packaging Status

- The repository includes prerelease Conan 2 and vcpkg package definitions.
- Neither package has been submitted to its central registry yet; that requires
  an immutable tagged release.
- Runtime UTF validation, UTF-8/UTF-16/UTF-32 transcoding, selected ASCII checks, and UTF-8/UTF-16 character counting use package-managed `simdutf` `8.2.0` or newer. A standalone CMake build fetches pinned `v8.2.0` when no package is already available.
- The standalone Visual Studio project retains a separate `simdutf` `v7.7.0` source snapshot; CMake, vcpkg, and Conan package builds do not compile that copy.
- The repository ships first-party Visual Studio and CMake build definitions for the compiled library target.

The practical source-based choices are:

1. Vendor a snapshot of the repository into your source tree.
2. Add the repository as a git submodule.
3. Fetch the repository in CMake and link the shipped `unicode_ranges` target.

Package-manager users can also create a Conan package from the checkout or use
the repository-owned vcpkg overlay port. See
[`packaging/README.md`](https://github.com/cristi1990an/unicode_ranges/blob/main/packaging/README.md)
for commands and CI coverage.

## What your build needs

- C++23 enabled
- the repository root on the include path
- the `unicode_ranges` library target built from `unicode_ranges.cpp`
- `#include "unicode_ranges_borrowed.hpp"` or `#include "unicode_ranges_all.hpp"` in user code
- an available `simdutf` CMake package (the first-party CMake build fetches pinned `v8.2.0` by default)

The public umbrella headers live at the repository root:

```cpp
#include "unicode_ranges_borrowed.hpp"
```

```cpp
#include "unicode_ranges_all.hpp"
```

Use:

- `unicode_ranges_borrowed.hpp` for the lighter borrowed/core surface
- `unicode_ranges_all.hpp` for the all-in umbrella, including owning strings and `unicode_ranges::characters`

## Runtime backend: simdutf

The compiled `unicode_ranges` library target uses `simdutf` for the hot runtime UTF boundary operations:

- UTF-8, UTF-16, and UTF-32 validation
- UTF-8, UTF-16, and UTF-32 transcoding on runtime paths
- UTF-8/UTF-16 character counting and selected ASCII-only checks

That is not accidental dependency creep. `simdutf` has been the strongest raw UTF codec baseline in the comparative benchmark suite, and using its public API lets `unicode_ranges` keep its own validated string/view/value types and error model while taking advantage of excellent runtime UTF performance.

The rest of the library remains `unicode_ranges` code:

- the public API surface
- validated UTF types
- grapheme logic
- normalization and casing layers
- compile-time and `constexpr`-oriented functionality

So the integration rule is simple:

- link `unicode_ranges` and preserve its transitive `simdutf` dependency
- let CMake fetch simdutf for a direct source build, or let vcpkg/Conan resolve it for a package build

## Recommended: vendor or submodule

If you vendor the repository or add it as a submodule, keep the checked-out tree intact:

```text
your_project/
  third_party/
    unicode_ranges/
      unicode_ranges.cpp
      unicode_ranges_borrowed.hpp
      unicode_ranges_all.hpp
      unicode_ranges.hpp
      unicode_ranges_full.hpp
      unicode_ranges/
```

For a direct CMake build, the project fetches simdutf if it is not already
installed. If integrating the target through another build system, provide a
simdutf package target (`simdutf::simdutf`) or use the vcpkg/Conan recipe.
Then build with:

- include directories:
  - `third_party/unicode_ranges`
- language mode: C++23
- one compiled library target:
  - compile `third_party/unicode_ranges/unicode_ranges.cpp` into `unicode_ranges`
  - link your executable or test target against that library

## Visual Studio

The repository contains first-party Visual Studio projects:

- `unicode_ranges.vcxproj`: static library
- `unicode_ranges_tests.vcxproj`: test runner linked against the library
- `tools/benchmarks/unicode_ranges_benchmarks.vcxproj`: benchmark runner linked against the library
- `tools/comparative_benchmarks/comparative_benchmarks.vcxproj`: comparative benchmark runner linked against the library

If you are consuming the repository directly from Visual Studio, build `unicode_ranges.vcxproj` and link it into your own executable or test project the same way the repo's test/benchmark projects do.

## CMake: first-party target

The repository ships a first-party CMake build, install, and package-export surface:

- target: `unicode_ranges::unicode_ranges`
- package config: `unicode_rangesConfig.cmake`
- install export under `lib/cmake/unicode_ranges`

The first-party CMake build also exposes linked test and benchmark executables.

If you add the repository with `add_subdirectory(...)`, just link the target:

```cmake
add_subdirectory(third_party/unicode_ranges)
target_link_libraries(your_target PRIVATE unicode_ranges::unicode_ranges)
```

## CMake: install / find_package

After configuring and installing the library:

```bash
cmake -S third_party/unicode_ranges -B build/unicode_ranges
cmake --build build/unicode_ranges
cmake --install build/unicode_ranges --prefix install/unicode_ranges
```

you can consume it as a normal package:

```cmake
find_package(unicode_ranges CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE unicode_ranges::unicode_ranges)
```

## CMake: FetchContent

If you prefer to fetch sources at configure time, fetch `unicode_ranges` and use the shipped target:

```cmake
include(FetchContent)

FetchContent_Declare(
    unicode_ranges_src
    GIT_REPOSITORY https://github.com/cristi1990an/unicode_ranges.git
    GIT_TAG <pinned-commit>
)

FetchContent_MakeAvailable(unicode_ranges_src)

target_link_libraries(your_target PRIVATE unicode_ranges::unicode_ranges)
```

This uses the first-party library target instead of rebuilding ad hoc target logic in your own project.

Do not track `main` in production builds. Pin an exact commit that you have validated in your own CI.

## Optional ICU-backed locale casing

The default library build depends on simdutf and exposes only locale-independent Unicode casing.

If you want ICU-backed locale-sensitive casing overloads such as `to_lowercase("tr"_locale)`, `to_uppercase("tr"_locale)`, or `case_fold("tr"_locale)`, enable the CMake option `UTF8_RANGES_ENABLE_ICU` and make ICU available to CMake. The build:

- find `ICU::uc` and `ICU::i18n`
- generates an installed config header that records ICU availability, so consumers do not need an extra compile definition
- link those ICU targets through `unicode_ranges::unicode_ranges`

If ICU is not found, the default build falls back to the locale-independent surface.

When ICU is enabled, locale-aware casing follows ICU locale resolution behavior. `locale_id` is a raw null-terminated locale-name token, while `_locale` gives you a compile-time checked literal form. The locale-aware overloads pass the token through to ICU, which may canonicalize it or fall back to a more general locale instead of failing. Use `is_available_locale(...)` when the active ICU data set must explicitly expose a locale before a locale-aware casing call.

## Licensing

`unicode_ranges` itself is dual-licensed under `MIT OR Apache-2.0`.

The generated Unicode tables are additionally distributed under Unicode
License V3. The CMake, vcpkg, and Conan packages use simdutf as a separate
package-managed dependency; its own package supplies its license notices.

For the exact repository licenses, third-party dependency versions, and notice policy, see:

- `LICENSE`
- `LICENSE-MIT`
- `LICENSE-APACHE`
- `LICENSE-UNICODE`
- `THIRD_PARTY_NOTICES.md`

## Toolchains exercised in CI

- MSVC with the MSVC STL: Visual Studio 2026 toolset `v145`
- Clang-cl with the MSVC STL: Visual Studio 2026 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

## Next steps

- [Getting Started](getting-started.md)
- [Common Tasks](common-tasks.md)
- [Design](design.md)
