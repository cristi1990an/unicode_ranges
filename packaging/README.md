# Package Manager Support

The repository provides prerelease package definitions for Conan 2 and vcpkg.
Both build the compiled static library, preserve the installed CMake package,
and expose ICU-backed locale behavior as an explicit opt-in feature.

## Conan 2

Create and test the package from the current checkout:

```bash
conan profile detect --force
conan create . --build=missing -s compiler.cppstd=23
```

Enable ICU with:

```bash
conan create . --build=missing -s compiler.cppstd=23 -o "&:with_icu=True"
```

`conan create` builds the recipe, packages its CMake install tree, and then
builds and runs `test_package/` as an independent consumer.

## vcpkg

`packaging/vcpkg/ports/unicode-ranges` is a repository-owned overlay port. It
packages the current checkout, which lets pull requests validate the port before
there is a release archive.

With a vcpkg checkout in `VCPKG_ROOT`, configure the test consumer with:

```bash
cmake -S packaging/vcpkg/test-project -B build/vcpkg-package-test \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_OVERLAY_PORTS="$PWD/packaging/vcpkg/ports"
cmake --build build/vcpkg-package-test
ctest --test-dir build/vcpkg-package-test --output-on-failure
```

Add `-DVCPKG_MANIFEST_FEATURES=icu` to validate the optional ICU feature.

The curated-registry port consumes a versioned GitHub release archive through
`vcpkg_from_github()` and pins its SHA512. The repository-owned overlay remains
available for testing the current checkout during development.

## Continuous integration

Package-manager checks are intentionally separate from the library's main CI:

- `.github/workflows/conan.yml` creates the Conan package and runs its test package.
- `.github/workflows/vcpkg.yml` installs the overlay port and runs a downstream
  manifest consumer.

The workflows cover Linux and Windows. Linux also exercises the optional ICU
feature.
