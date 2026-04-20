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

SIMDUTF_ROOT="${SIMDUTF_ROOT:-.local/comparative-deps-assets/simdutf}"
if [[ ! -f "${SIMDUTF_ROOT}/simdutf.h" || ! -f "${SIMDUTF_ROOT}/simdutf.cpp" ]]; then
  echo "SIMDUTF_ROOT must point to a simdutf singleheader release containing simdutf.h and simdutf.cpp." >&2
  exit 1
fi

runtime_obj="$out_dir/unicode_ranges_runtime.o"
"$cxx" -std=c++23 -I. -I"${SIMDUTF_ROOT}" "$@" -c unicode_ranges.cpp -o "$runtime_obj"

mapfile -t sources < <(find docs/examples -name '*.cpp' | sort)

for source in "${sources[@]}"; do
  rel="${source#docs/examples/}"
  stem="${rel%.cpp}"
  exe_name="${stem//\//__}"
  echo "Compiling $source"
  "$cxx" -std=c++23 -I. -I"${SIMDUTF_ROOT}" "$@" "$source" "$runtime_obj" -o "$out_dir/$exe_name"
done
