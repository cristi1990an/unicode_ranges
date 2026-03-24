#!/usr/bin/env bash

set -euo pipefail

CXX="${CXX:-clang++-22}"
LLVM_COV="${LLVM_COV:-llvm-cov-22}"
LLVM_PROFDATA="${LLVM_PROFDATA:-llvm-profdata-22}"

OUTPUT_DIR="${OUTPUT_DIR:-coverage}"
BINARY="${BINARY:-unicode_ranges_cov}"
PROFRAW="${OUTPUT_DIR}/unicode_ranges.profraw"
PROFDATA="${OUTPUT_DIR}/unicode_ranges.profdata"
REPORT_TXT="${OUTPUT_DIR}/report.txt"
HTML_DIR="${OUTPUT_DIR}/html"

mkdir -p "${OUTPUT_DIR}"

"${CXX}" \
	-std=c++23 \
	-O0 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-stdlib=libc++ \
	-fprofile-instr-generate \
	-fcoverage-mapping \
	-fno-inline \
	source.cpp \
	-o "${OUTPUT_DIR}/${BINARY}"

LLVM_PROFILE_FILE="${PROFRAW}" "${OUTPUT_DIR}/${BINARY}"

"${LLVM_PROFDATA}" merge -sparse "${PROFRAW}" -o "${PROFDATA}"

mapfile -t COVERED_FILES < <(find unicode_ranges -name '*.hpp' -print | sort)
COVERED_FILES+=("unicode_ranges.hpp")

"${LLVM_COV}" report \
	"${OUTPUT_DIR}/${BINARY}" \
	-instr-profile="${PROFDATA}" \
	-show-functions \
	-show-branch-summary \
	"${COVERED_FILES[@]}" | tee "${REPORT_TXT}"

"${LLVM_COV}" show \
	"${OUTPUT_DIR}/${BINARY}" \
	-instr-profile="${PROFDATA}" \
	-show-line-counts-or-regions \
	-show-branches=count \
	-format=html \
	-output-dir "${HTML_DIR}" \
	"${COVERED_FILES[@]}"
