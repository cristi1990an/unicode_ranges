#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "usage: $0 <compiler> <output-dir> [extra compiler flags...]" >&2
  exit 1
fi

cxx=$1
shift
out_dir=$1
shift

mkdir -p "$out_dir"

if [[ ! -f "third_party/simdutf/simdutf.h" || ! -f "third_party/simdutf/simdutf.cpp" ]]; then
  echo "Vendored simdutf is missing from third_party/simdutf." >&2
  exit 1
fi

runtime_obj="$out_dir/unicode_ranges_runtime.o"
tables_obj="$out_dir/unicode_tables_runtime.o"
simdutf_obj="$out_dir/simdutf.o"
runtime_lib="$out_dir/libunicode_ranges.a"
"$cxx" -std=c++14 -Ithird_party/simdutf "$@" -Wno-error -c third_party/simdutf/simdutf.cpp -o "$simdutf_obj"
"$cxx" -std=c++23 -I. -Ithird_party/simdutf "$@" -Wno-error=overflow -Wno-error=pedantic -c unicode_ranges.cpp -o "$runtime_obj"
"$cxx" -std=c++23 -I. -Ithird_party/simdutf "$@" -Wno-error=overflow -Wno-error=pedantic -c unicode_tables_runtime.cpp -o "$tables_obj"
ar rcs "$runtime_lib" "$runtime_obj" "$tables_obj" "$simdutf_obj"

mapfile -t sources < <(find docs/examples -name '*.cpp' | sort)

for source in "${sources[@]}"; do
  rel="${source#docs/examples/}"
  stem="${rel%.cpp}"
  exe_name="${stem//\//__}"
  echo "Compiling $source"
  "$cxx" -std=c++23 -I. "$@" "$source" "$runtime_lib" -o "$out_dir/$exe_name"
done
