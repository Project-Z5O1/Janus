#!/bin/bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  preprocess_apk.sh --apk <apk_path> --package <package> [--ghidra-jobs <n>] [--keep-temp]

Description:
  Run Phase 1 source preprocessing for a single APK package.
  - Runs JADX and apktool in parallel
  - Exports native decompilation with one bounded-concurrency Ghidra job per .so
  - Validates outputs before replacing target_APK/<package>/src/*

Options:
  --apk <apk_path>       APK file to preprocess
  --package <package>    Package directory name under target_APK/
  --ghidra-jobs <n>      Parallel Ghidra exports. Defaults to min(cpu_count, 4)
  --keep-temp            Keep temporary workspace and logs after success
  -h, --help             Show this message
EOF
}

log() {
  printf '[%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*"
}

die() {
  printf 'Error: %s\n' "$*" >&2
  exit 1
}

require_cmd() {
  local cmd="$1"
  command -v "$cmd" >/dev/null 2>&1 || die "missing required command: $cmd"
}

resolve_path() {
  local input_path="$1"
  if [ -d "$input_path" ]; then
    (cd "$input_path" && pwd)
    return
  fi
  (cd "$(dirname "$input_path")" && printf '%s/%s\n' "$(pwd)" "$(basename "$input_path")")
}

default_ghidra_jobs() {
  local cpu_count="1"
  if command -v getconf >/dev/null 2>&1; then
    cpu_count="$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '1')"
  elif command -v sysctl >/dev/null 2>&1; then
    cpu_count="$(sysctl -n hw.ncpu 2>/dev/null || printf '1')"
  fi
  case "$cpu_count" in
    ''|*[!0-9]*)
      cpu_count="1"
      ;;
  esac
  if [ "$cpu_count" -gt 4 ]; then
    cpu_count="4"
  fi
  if [ "$cpu_count" -lt 1 ]; then
    cpu_count="1"
  fi
  printf '%s\n' "$cpu_count"
}

sanitize_name() {
  printf '%s' "$1" | tr -c 'A-Za-z0-9' '_'
}

apk_path=""
package=""
ghidra_jobs="$(default_ghidra_jobs)"
keep_temp=0

while [ "$#" -gt 0 ]; do
  case "$1" in
    --apk)
      [ "$#" -ge 2 ] || die "--apk requires a value"
      apk_path="$2"
      shift 2
      ;;
    --package)
      [ "$#" -ge 2 ] || die "--package requires a value"
      package="$2"
      shift 2
      ;;
    --ghidra-jobs)
      [ "$#" -ge 2 ] || die "--ghidra-jobs requires a value"
      ghidra_jobs="$2"
      shift 2
      ;;
    --keep-temp)
      keep_temp=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      die "unknown argument: $1"
      ;;
  esac
done

[ -n "$apk_path" ] || {
  usage >&2
  die "missing --apk"
}
[ -n "$package" ] || {
  usage >&2
  die "missing --package"
}
case "$ghidra_jobs" in
  ''|*[!0-9]*)
    die "--ghidra-jobs must be a positive integer"
    ;;
esac
[ "$ghidra_jobs" -ge 1 ] || die "--ghidra-jobs must be at least 1"
[ -f "$apk_path" ] || die "APK file does not exist: $apk_path"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../../../.." && pwd)"
apk_abs="$(resolve_path "$apk_path")"
package_dir="$repo_root/target_APK/$package"
src_dir="$package_dir/src"
tmp_root="$package_dir/.phase1_tmp.$$"
tmp_src_dir="$tmp_root/src"
java_out="$tmp_src_dir/java"
smali_out="$tmp_src_dir/smali"
native_out="$tmp_src_dir/native"
log_dir="$tmp_root/logs"

jadx_bin="$repo_root/retool/jadx-1.5.4/bin/jadx"
apktool_jar="$repo_root/retool/apktool_3.0.1.jar"
ghidra_headless="$repo_root/retool/ghidra_12.0.3_PUBLIC/support/analyzeHeadless"
ghidra_script_path="$repo_root/retool/ghidra-scripts"
ghidra_export_script="$repo_root/retool/ghidra-scripts/ExportSoDecomp.java"

preserve_tmp=1
cleanup_tmp() {
  if [ "$preserve_tmp" -eq 0 ] && [ -d "$tmp_root" ]; then
    rm -rf "$tmp_root"
  fi
}
trap cleanup_tmp EXIT

require_cmd java
require_cmd unzip
require_cmd mktemp
[ -x "$jadx_bin" ] || die "missing JADX binary: $jadx_bin"
[ -f "$apktool_jar" ] || die "missing apktool jar: $apktool_jar"
[ -x "$ghidra_headless" ] || die "missing Ghidra headless launcher: $ghidra_headless"
[ -f "$ghidra_export_script" ] || die "missing Ghidra export script: $ghidra_export_script"

mkdir -p "$package_dir" "$java_out" "$smali_out" "$native_out" "$log_dir"

if [ -z "$(find "$package_dir/lib" -type f -name '*.so' -print -quit 2>/dev/null)" ]; then
  log "No extracted native libraries found under target_APK/$package/lib; extracting from APK"
  unzip -o "$apk_abs" "lib/*/*.so" -d "$package_dir" >/dev/null
fi

declare -a so_files=()
while IFS= read -r so_path; do
  so_files+=("$so_path")
done < <(find "$package_dir/lib" -type f -name '*.so' | sort)

[ "${#so_files[@]}" -gt 0 ] || die "no native libraries found under $package_dir/lib"

run_jadx() {
  local jadx_rc
  log "Starting JADX export"
  set +e
  "$jadx_bin" --no-res --show-bad-code -d "$java_out" "$apk_abs"
  jadx_rc="$?"
  set -e

  if [ "$jadx_rc" -eq 0 ]; then
    return 0
  fi

  if [ -n "$(find "$java_out" -type f -print -quit 2>/dev/null)" ]; then
    log "JADX exited with code=$jadx_rc but produced source output; continuing"
    return 0
  fi

  return "$jadx_rc"
}

run_apktool() {
  log "Starting apktool decode"
  java -jar "$apktool_jar" d -f --no-res -o "$smali_out" "$apk_abs"
}

run_single_ghidra() {
  local so_path="$1"
  local abi
  local so_name
  local so_base
  local abi_safe
  local base_safe
  local package_safe
  local out_dir
  local out_file
  local project_root
  local project_name
  local ghidra_log

  abi="$(basename "$(dirname "$so_path")")"
  so_name="$(basename "$so_path")"
  so_base="${so_name%.so}"
  abi_safe="$(sanitize_name "$abi")"
  base_safe="$(sanitize_name "$so_base")"
  package_safe="$(sanitize_name "$package")"
  out_dir="$native_out/$abi"
  out_file="$out_dir/${so_base}.c"
  ghidra_log="$log_dir/ghidra_${abi_safe}_${base_safe}.log"

  mkdir -p "$out_dir"
  project_root="$(mktemp -d "${TMPDIR:-/tmp}/omen_ghidra_${package_safe}_${abi_safe}_${base_safe}_XXXXXX")"
  project_name="export_${base_safe}"

  if ! "$ghidra_headless" \
      "$project_root" "$project_name" \
      -import "$so_path" \
      -scriptPath "$ghidra_script_path" \
      -postScript "ExportSoDecomp.java" "$out_file" \
      -deleteProject >"$ghidra_log" 2>&1; then
    rm -rf "$project_root"
    printf 'Ghidra export failed for %s. See %s\n' "$so_name" "$ghidra_log" >&2
    return 1
  fi

  rm -rf "$project_root"

  if [ ! -s "$out_file" ]; then
    printf 'Ghidra export produced no output for %s. See %s\n' "$so_name" "$ghidra_log" >&2
    return 1
  fi
}

run_native_exports() {
  local native_failed=0
  local -a running_pids=()
  local so_path
  local pid

  log "Starting Ghidra native export for ${#so_files[@]} libraries with concurrency=$ghidra_jobs"
  for so_path in "${so_files[@]}"; do
    run_single_ghidra "$so_path" &
    running_pids+=("$!")
    if [ "${#running_pids[@]}" -ge "$ghidra_jobs" ]; then
      pid="${running_pids[0]}"
      if ! wait "$pid"; then
        native_failed=1
      fi
      running_pids=("${running_pids[@]:1}")
    fi
  done

  for pid in "${running_pids[@]}"; do
    if ! wait "$pid"; then
      native_failed=1
    fi
  done

  return "$native_failed"
}

validate_outputs() {
  local so_path
  local abi
  local so_name
  local so_base
  local out_file

  [ -n "$(find "$java_out" -type f -print -quit 2>/dev/null)" ] || die "JADX output is empty"
  [ -n "$(find "$smali_out" -type f -name '*.smali' -print -quit 2>/dev/null)" ] || die "apktool output has no .smali files"

  for so_path in "${so_files[@]}"; do
    abi="$(basename "$(dirname "$so_path")")"
    so_name="$(basename "$so_path")"
    so_base="${so_name%.so}"
    out_file="$native_out/$abi/${so_base}.c"
    [ -s "$out_file" ] || die "missing native export for $so_name at $out_file"
  done
}

log "Phase 1 preprocess start for package=$package"
log "APK=$apk_abs"
log "Ghidra jobs=$ghidra_jobs"

"run_jadx" >"$log_dir/jadx.log" 2>&1 &
jadx_pid="$!"

"run_apktool" >"$log_dir/apktool.log" 2>&1 &
apktool_pid="$!"

"run_native_exports" >"$log_dir/native.log" 2>&1 &
native_pid="$!"

phase_failed=0

if ! wait "$jadx_pid"; then
  printf 'JADX export failed. See %s\n' "$log_dir/jadx.log" >&2
  phase_failed=1
fi

if ! wait "$apktool_pid"; then
  printf 'apktool decode failed. See %s\n' "$log_dir/apktool.log" >&2
  phase_failed=1
fi

if ! wait "$native_pid"; then
  printf 'Native export failed. See %s and per-lib ghidra logs under %s\n' "$log_dir/native.log" "$log_dir" >&2
  phase_failed=1
fi

[ "$phase_failed" -eq 0 ] || die "Phase 1 preprocess failed; temporary workspace kept at $tmp_root"

validate_outputs

mkdir -p "$src_dir"
rm -rf "$src_dir/java" "$src_dir/smali" "$src_dir/native"
mv "$java_out" "$src_dir/java"
mv "$smali_out" "$src_dir/smali"
mv "$native_out" "$src_dir/native"

java_count="$(find "$src_dir/java" -type f | wc -l | awk '{print $1}')"
smali_count="$(find "$src_dir/smali" -type f -name '*.smali' | wc -l | awk '{print $1}')"
native_count="$(find "$src_dir/native" -type f -name '*.c' | wc -l | awk '{print $1}')"

log "Phase 1 preprocess complete for package=$package"
log "Java files=$java_count Smali files=$smali_count Native exports=$native_count"

if [ "$keep_temp" -eq 0 ]; then
  preserve_tmp=0
else
  log "Keeping temporary workspace at $tmp_root"
fi
