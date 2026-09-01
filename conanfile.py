import os
import re

from conan import ConanFile
from conan.errors import ConanException, ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import load
from conan.tools.scm import Version


class UnicodeRangesConan(ConanFile):
    name = "unicode-ranges"
    package_type = "static-library"
    license = "MIT OR Apache-2.0"
    url = "https://github.com/cristi1990an/unicode_ranges"
    homepage = "https://cristi1990an.github.io/unicode_ranges/"
    description = (
        "C++23 validated UTF-8, UTF-16, and UTF-32 text types and algorithms"
    )
    topics = ("unicode", "utf-8", "utf-16", "utf-32", "text")

    required_conan_version = ">=2.20"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "fPIC": [True, False],
        "with_icu": [True, False],
    }
    default_options = {
        "fPIC": True,
        "with_icu": False,
    }

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "unicode_ranges.cpp",
        "unicode_tables_runtime.cpp",
        "unicode_ranges*.hpp",
        "unicode_ranges/*",
        "third_party/simdutf/*",
        "LICENSE*",
        "THIRD_PARTY_NOTICES.md",
        "README.md",
    )

    def set_version(self):
        if self.version:
            return

        cmake_contents = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
        match = re.search(
            r"project\(unicode_ranges\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)",
            cmake_contents,
        )
        if not match:
            raise ConanException("Could not determine the package version from CMakeLists.txt")

        self.version = match.group(1)

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def requirements(self):
        if self.options.with_icu:
            self.requires("icu/78.2")

    def validate(self):
        check_min_cppstd(self, "23")

        compiler = str(self.settings.compiler)
        minimum_versions = {
            "gcc": "14",
            "clang": "22",
            "msvc": "195",
        }
        minimum = minimum_versions.get(compiler)
        if minimum and Version(self.settings.compiler.version) < minimum:
            raise ConanInvalidConfiguration(
                f"unicode-ranges requires {compiler} {minimum} or newer"
            )

    def layout(self):
        cmake_layout(self)

    def generate(self):
        dependencies = CMakeDeps(self)
        dependencies.generate()

        toolchain = CMakeToolchain(self)
        toolchain.variables["UTF8_RANGES_BUILD_TESTS"] = False
        toolchain.variables["UTF8_RANGES_BUILD_BENCHMARKS"] = False
        toolchain.variables["UTF8_RANGES_ENABLE_ICU"] = bool(self.options.with_icu)
        toolchain.variables["CMAKE_FIND_PACKAGE_PREFER_CONFIG"] = True
        if self.options.get_safe("fPIC") is not None:
            toolchain.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = bool(
                self.options.fPIC
            )
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "unicode_ranges")
        self.cpp_info.set_property(
            "cmake_target_name", "unicode_ranges::unicode_ranges"
        )
        self.cpp_info.libs = ["unicode_ranges"]

        if self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs.append("pthread")

        if self.options.with_icu:
            self.cpp_info.defines.append("UTF8_RANGES_ENABLE_ICU=1")
            self.cpp_info.requires.extend(("icu::icu-uc", "icu::icu-i18n"))
