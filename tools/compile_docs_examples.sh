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

simdutf_candidates=()
if [[ -n "${SIMDUTF_ROOT:-}" ]]; then
  simdutf_candidates+=("${SIMDUTF_ROOT}")
fi
simdutf_candidates+=(
  "build/runtime-deps/simdutf"
  ".local/comparative-deps-assets/simdutf"
)

resolved_simdutf_root=""
for candidate in "${simdutf_candidates[@]}"; do
  if [[ -f "${candidate}/simdutf.h" && -f "${candidate}/simdutf.cpp" ]]; then
    resolved_simdutf_root="$candidate"
    break
  fi
done

if [[ -z "$resolved_simdutf_root" ]]; then
  echo "SIMDUTF_ROOT must point to a simdutf singleheader release containing simdutf.h and simdutf.cpp." >&2
  exit 1
fi

runtime_obj="$out_dir/unicode_ranges_runtime.o"
"$cxx" -std=c++23 -I. -I"${resolved_simdutf_root}" "$@" -Wno-error=overflow -Wno-error=pedantic -c unicode_ranges.cpp -o "$runtime_obj"

mapfile -t sources < <(find docs/examples -name '*.cpp' | sort)

for source in "${sources[@]}"; do
  rel="${source#docs/examples/}"
  stem="${rel%.cpp}"
  exe_name="${stem//\//__}"
  echo "Compiling $source"
  "$cxx" -std=c++23 -I. -I"${resolved_simdutf_root}" "$@" "$source" "$runtime_obj" -o "$out_dir/$exe_name"
done
