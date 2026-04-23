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

round_count="${UTF8_RANGES_COMPILE_TIME_ROUNDS:-5}"

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

printf "| Probe | Median Seconds | Samples |\n" > "$summary_file"
printf "| --- | ---: | --- |\n" >> "$summary_file"
printf "probe,round,seconds\n" > "$csv_file"

measure_probe() {
  local probe_name="$1"
  local source_path="$2"
  local round_index="$3"
  local object_path="$output_dir/${probe_name}.r${round_index}.o"
  local time_path="$output_dir/${probe_name}.r${round_index}.time"
  local seconds

  /usr/bin/time -f "%e" -o "$time_path" \
    "$cxx" "${compile_flags[@]}" "$source_path" -o "$object_path"

  seconds="$(tr -d '\r\n' < "$time_path")"
  printf "%s,%s,%s\n" "$probe_name" "$round_index" "$seconds" >> "$csv_file"
}

median_for_probe() {
  local probe_name="$1"
  awk -F, -v target="$probe_name" '
    NR > 1 && $1 == target { print $3 }
  ' "$csv_file" | sort -n | awk '
    {
      values[NR] = $1
    }
    END {
      if (NR == 0) {
        exit 1
      }
      if (NR % 2 == 1) {
        print values[(NR + 1) / 2]
      } else {
        low = values[NR / 2]
        high = values[(NR / 2) + 1]
        printf "%.2f\n", (low + high) / 2.0
      }
    }
  '
}

samples_for_probe() {
  local probe_name="$1"
  awk -F, -v target="$probe_name" '
    NR > 1 && $1 == target {
      values[count++] = $3
    }
    END {
      for (i = 0; i < count; ++i) {
        if (i != 0) {
          printf ", "
        }
        printf "%s", values[i]
      }
    }
  ' "$csv_file"
}

probe_count="${#probes[@]}"

for ((round_index = 0; round_index < round_count; ++round_index)); do
  offset=$((round_index % probe_count))
  for ((probe_index = 0; probe_index < probe_count; ++probe_index)); do
    current_index=$(((probe_index + offset) % probe_count))
    probe="${probes[current_index]}"
    probe_name="${probe%%:*}"
    source_path="${probe#*:}"
    measure_probe "$probe_name" "$source_path" "$round_index"
  done
done

for probe in "${probes[@]}"; do
  probe_name="${probe%%:*}"
  median_seconds="$(median_for_probe "$probe_name")"
  samples="$(samples_for_probe "$probe_name")"
  printf "| %s | %s | %s |\n" "$probe_name" "$median_seconds" "$samples" >> "$summary_file"
done
