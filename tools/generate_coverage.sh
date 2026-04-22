#!/usr/bin/env bash

set -euo pipefail

CXX="${CXX:-clang++-22}"
LLVM_COV="${LLVM_COV:-llvm-cov-22}"
LLVM_PROFDATA="${LLVM_PROFDATA:-llvm-profdata-22}"

OUTPUT_DIR="${OUTPUT_DIR:-coverage}"
BINARY="${BINARY:-unicode_ranges_cov}"
BENCH_BINARY="${BENCH_BINARY:-unicode_ranges_cov_bench}"
PROFRAW="${OUTPUT_DIR}/unicode_ranges.profraw"
PROFRAW_BENCH="${OUTPUT_DIR}/unicode_ranges_bench.profraw"
PROFDATA="${OUTPUT_DIR}/unicode_ranges.profdata"
REPORT_TXT="${OUTPUT_DIR}/report.txt"
SUMMARY_TXT="${OUTPUT_DIR}/summary.txt"
HTML_DIR="${OUTPUT_DIR}/html"

mkdir -p "${OUTPUT_DIR}"

if [[ ! -f "third_party/simdutf/simdutf.h" || ! -f "third_party/simdutf/simdutf.cpp" ]]; then
	echo "Vendored simdutf is missing from third_party/simdutf." >&2
	exit 1
fi

"${CXX}" \
	-std=c++23 \
	-O0 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-pthread \
	-stdlib=libc++ \
	-Wno-error=overflow \
	-Wno-error=pedantic \
	-DUTF8_RANGES_ENABLE_TEST_HOOKS=1 \
	-DUTF8_RANGES_TEST_FORCE_UTF32_PARALLEL=1 \
	-fprofile-instr-generate \
	-fcoverage-mapping \
	-fno-inline \
	-c \
	unicode_ranges.cpp \
	-o "${OUTPUT_DIR}/unicode_ranges_runtime.o"

"${CXX}" \
	-std=c++23 \
	-O0 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-pthread \
	-stdlib=libc++ \
	-Wno-error=overflow \
	-Wno-error=pedantic \
	-DUTF8_RANGES_ENABLE_TEST_HOOKS=1 \
	-DUTF8_RANGES_TEST_FORCE_UTF32_PARALLEL=1 \
	-fprofile-instr-generate \
	-fcoverage-mapping \
	-fno-inline \
	-c \
	unicode_tables_runtime.cpp \
	-o "${OUTPUT_DIR}/unicode_tables_runtime.o"

ar rcs "${OUTPUT_DIR}/libunicode_ranges.a" "${OUTPUT_DIR}/unicode_ranges_runtime.o" "${OUTPUT_DIR}/unicode_tables_runtime.o"

"${CXX}" \
	-std=c++23 \
	-O0 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-pthread \
	-stdlib=libc++ \
	-DUTF8_RANGES_ENABLE_TEST_HOOKS=1 \
	-DUTF8_RANGES_TEST_FORCE_UTF32_PARALLEL=1 \
	-fprofile-instr-generate \
	-fcoverage-mapping \
	-fno-inline \
	source.cpp "${OUTPUT_DIR}/libunicode_ranges.a" \
	-o "${OUTPUT_DIR}/${BINARY}"

LLVM_PROFILE_FILE="${PROFRAW}" "${OUTPUT_DIR}/${BINARY}"

"${CXX}" \
	-std=c++23 \
	-O0 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-pthread \
	-stdlib=libc++ \
	-DUTF8_RANGES_ENABLE_TEST_HOOKS=1 \
	-DUTF8_RANGES_TEST_FORCE_UTF32_PARALLEL=1 \
	-fprofile-instr-generate \
	-fcoverage-mapping \
	-fno-inline \
	tools/benchmarks/unicode_ranges_benchmarks.cpp "${OUTPUT_DIR}/libunicode_ranges.a" \
	-o "${OUTPUT_DIR}/${BENCH_BINARY}"

LLVM_PROFILE_FILE="${PROFRAW_BENCH}" "${OUTPUT_DIR}/${BENCH_BINARY}" --quick --filter=large.view

"${LLVM_PROFDATA}" merge -sparse "${PROFRAW}" "${PROFRAW_BENCH}" -o "${PROFDATA}"

mapfile -t COVERED_FILES < <(find unicode_ranges -name '*.hpp' -print | sort)
COVERED_FILES+=("unicode_ranges.hpp")

"${LLVM_COV}" report \
	"${OUTPUT_DIR}/${BINARY}" \
	-instr-profile="${PROFDATA}" \
	-show-functions \
	-show-branch-summary \
	"${COVERED_FILES[@]}" | tee "${REPORT_TXT}"

"${LLVM_COV}" report \
	"${OUTPUT_DIR}/${BINARY}" \
	-instr-profile="${PROFDATA}" \
	-show-branch-summary \
	"${COVERED_FILES[@]}" | tee "${SUMMARY_TXT}"

"${LLVM_COV}" show \
	"${OUTPUT_DIR}/${BINARY}" \
	-instr-profile="${PROFDATA}" \
	-show-line-counts-or-regions \
	-show-branches=count \
	-format=html \
	-output-dir "${HTML_DIR}" \
	"${COVERED_FILES[@]}"
