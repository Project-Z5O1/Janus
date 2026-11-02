#!/bin/bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  preprocess_apk.sh --apk <apk_path> --package <package> [--device <serial>] [--ghidra-jobs <n>] [--keep-temp]

Description:
  Run the complete Cross-Language Parser for one APK.
  - Resolve JNI bindings on device with a per-run remote namespace
  - Build jni_list.json from the extractor's structured entries
  - Run JADX and apktool in parallel
  - Export bounded-concurrency Ghidra decompilation with isolated projects
  - Validate outputs before replacing target_APK/<package>/src/*
  - Initialize or resume summary.json schema v2 through omen_state.py

Options:
  --apk <apk_path>       APK file to preprocess
  --package <package>    Package directory name under target_APK/
  --device <serial>      ADB device serial; defaults to OMEN_DEVICE_ID
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

find_repo_root() {
  local script_dir="$1"
  local candidate
  if [ -n "${OMEN_ROOT:-}" ]; then
    (cd "$OMEN_ROOT" && pwd)
    return
  fi
  candidate="$script_dir"
  while [ "$candidate" != "/" ]; do
    if [ -f "$candidate/AGENTS.md" ] && [ -d "$candidate/retool" ]; then
      printf '%s\n' "$candidate"
      return
    fi
    candidate="$(dirname "$candidate")"
  done
  die "cannot locate repository root; set OMEN_ROOT"
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
device_id="${OMEN_DEVICE_ID:-}"
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
    --device)
      [ "$#" -ge 2 ] || die "--device requires a value"
      device_id="$2"
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
[ -n "$device_id" ] || {
  usage >&2
  die "missing --device and OMEN_DEVICE_ID"
}
case "$package" in
  ''|'.'|'..'|*'/'*|*"\\"*) die "invalid package name: $package" ;;
esac
case "$ghidra_jobs" in
  ''|*[!0-9]*)
    die "--ghidra-jobs must be a positive integer"
    ;;
esac
[ "$ghidra_jobs" -ge 1 ] || die "--ghidra-jobs must be at least 1"
[ -f "$apk_path" ] || die "APK file does not exist: $apk_path"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(find_repo_root "$script_dir")"
apk_abs="$(resolve_path "$apk_path")"
package_dir="$repo_root/target_APK/$package"
src_dir="$package_dir/src"
tmp_root="$package_dir/.parser_tmp.$$"
tmp_src_dir="$tmp_root/src"
java_out="$tmp_src_dir/java"
smali_out="$tmp_src_dir/smali"
native_out="$tmp_src_dir/native"
log_dir="$tmp_root/logs"
persistent_log_dir="$package_dir/logs/parser"

jni_extractor="$repo_root/mcptool/jniinfo.py"
binding_builder="$script_dir/build_binding_table.py"
state_helper="$repo_root/.agents/skills/omen-pipeline/scripts/omen_state.py"

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
require_cmd python3
[ -x "$jadx_bin" ] || die "missing JADX binary: $jadx_bin"
[ -f "$apktool_jar" ] || die "missing apktool jar: $apktool_jar"
[ -x "$ghidra_headless" ] || die "missing Ghidra headless launcher: $ghidra_headless"
[ -f "$ghidra_export_script" ] || die "missing Ghidra export script: $ghidra_export_script"
[ -f "$jni_extractor" ] || die "missing JNI extractor: $jni_extractor"
[ -f "$binding_builder" ] || die "missing binding table builder: $binding_builder"
[ -f "$state_helper" ] || die "missing state helper: $state_helper"

mkdir -p "$package_dir" "$java_out" "$smali_out" "$native_out" "$log_dir" "$persistent_log_dir"

if [ "$apk_abs" != "$package_dir/base.apk" ]; then
  cp "$apk_abs" "$package_dir/base.apk"
fi

log "Resolving JNI bindings on device=$device_id"
if ! python3 "$jni_extractor" extract \
    --apk "$apk_abs" \
    --package "$package" \
    --device "$device_id" \
    --output-dir "$repo_root/target_APK" \
    >"$persistent_log_dir/jni_extract.json" 2>"$persistent_log_dir/jni_extract.stderr.log"; then
  die "JNI binding extraction failed; see $persistent_log_dir/jni_extract.stderr.log"
fi
python3 "$binding_builder" \
  --extract-result "$persistent_log_dir/jni_extract.json" \
  --package "$package" \
  --output "$package_dir/jni_list.json"

if [ -z "$(find "$package_dir/lib" -type f -name '*.so' -print -quit 2>/dev/null)" ]; then
  log "No extracted native libraries found under target_APK/$package/lib; extracting from APK"
  unzip -o "$apk_abs" "lib/*/*.so" -d "$package_dir" >/dev/null
fi

declare -a so_files=()
while IFS= read -r so_path; do
  so_files+=("$so_path")
done < <(find "$package_dir/lib" -type f -name '*.so' | sort)

# Some packed SDKs ship non-ELF payloads disguised as .so (e.g. Alibaba
# security shells: a ZIP/APK that a loader unpacks at runtime). Ghidra cannot
# import them and no JNI binding ever resolves into them; skip them with an
# explicit log line instead of failing the whole native export.
declare -a skipped_non_elf=()
filtered_so_files=()
for so_path in "${so_files[@]}"; do
  if head -c 4 "$so_path" | grep -q $'\x7fELF' 2>/dev/null; then
    filtered_so_files+=("$so_path")
  else
    skipped_non_elf+=("$(basename "$so_path")")
  fi
done
if [ "${#skipped_non_elf[@]}" -gt 0 ]; then
  log "Skipping non-ELF .so payloads (packed SDK shells): ${skipped_non_elf[*]}"
fi
so_files=("${filtered_so_files[@]}")

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

log "Parser preprocess start for package=$package"
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

[ "$phase_failed" -eq 0 ] || die "Parser preprocess failed; temporary workspace kept at $tmp_root"

validate_outputs

mkdir -p "$src_dir"
rm -rf "$src_dir/java" "$src_dir/smali" "$src_dir/native"
mv "$java_out" "$src_dir/java"
mv "$smali_out" "$src_dir/smali"
mv "$native_out" "$src_dir/native"

java_count="$(find "$src_dir/java" -type f | wc -l | awk '{print $1}')"
smali_count="$(find "$src_dir/smali" -type f -name '*.smali' | wc -l | awk '{print $1}')"
native_count="$(find "$src_dir/native" -type f -name '*.c' | wc -l | awk '{print $1}')"

python3 "$state_helper" --repo-root "$repo_root" init \
  --package "$package" \
  --jni-list "$package_dir/jni_list.json" \
  --apk "$apk_abs" \
  >"$persistent_log_dir/state_init.json"

log "Parser preprocess complete for package=$package"
log "Java files=$java_count Smali files=$smali_count Native exports=$native_count"

if [ "$keep_temp" -eq 0 ]; then
  preserve_tmp=0
else
  log "Keeping temporary workspace at $tmp_root"
fi
