#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 3 ]; then
  echo "usage: $0 <cxx> <output-dir> <summary-file> [compiler-flags...]" >&2
  exit 1
fi

cxx="$1"
shift
output_dir="$1"
shift
summary_file="$1"
shift

mkdir -p "$output_dir"

declare -a compile_flags=(
  -std=c++23
  -I.
  -c
  "$@"
)

declare -a probes=(
  "borrowed:tools/compile_time/probes/borrowed.cpp"
  "all:tools/compile_time/probes/all.cpp"
  "owning_runtime:tools/compile_time/probes/owning_runtime.cpp"
)

csv_file="$output_dir/results.csv"

printf "| Probe | Seconds |\n" > "$summary_file"
printf "| --- | ---: |\n" >> "$summary_file"
printf "probe,seconds\n" > "$csv_file"

measure_probe() {
  local probe_name="$1"
  local source_path="$2"
  local object_path="$output_dir/${probe_name}.o"
  local time_path="$output_dir/${probe_name}.time"
  local seconds

  /usr/bin/time -f "%e" -o "$time_path" \
    "$cxx" "${compile_flags[@]}" "$source_path" -o "$object_path"

  seconds="$(tr -d '\r\n' < "$time_path")"

  printf "| %s | %s |\n" "$probe_name" "$seconds" >> "$summary_file"
  printf "%s,%s\n" "$probe_name" "$seconds" >> "$csv_file"
}

for probe in "${probes[@]}"; do
  probe_name="${probe%%:*}"
  source_path="${probe#*:}"
  measure_probe "$probe_name" "$source_path"
done
