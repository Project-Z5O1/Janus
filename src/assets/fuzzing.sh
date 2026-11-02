#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_DIR="$(dirname "$SCRIPT_DIR")"
PACKAGE="$(basename "$PACKAGE_DIR")"
JNI_FUNC="$(basename "$SCRIPT_DIR")"
PROFILE="${OMEN_PROFILE:-release}"
DEVICE_DIR="/data/local/tmp/omen/$JNI_FUNC"
LOCK_DIR="/data/local/tmp/omen/.fuzzing.lock"
DURATION="${OMEN_FUZZ_TIMEOUT:-240}"

if [ "$PROFILE" = "debug" ]; then
  BUILD_DIR="$SCRIPT_DIR/build-debug"
  AFL_SCRIPT="$SCRIPT_DIR/afl-debug.js"
else
  PROFILE="release"
  BUILD_DIR="$SCRIPT_DIR/build"
  AFL_SCRIPT="$SCRIPT_DIR/afl.js"
fi

candidate="$SCRIPT_DIR"
REPO_ROOT=""
while [ "$candidate" != "/" ]; do
  if [ -f "$candidate/.agents/skills/omen-pipeline/scripts/preflight.py" ]; then
    REPO_ROOT="$candidate"
    break
  fi
  candidate="$(dirname "$candidate")"
done
if [ -z "$REPO_ROOT" ]; then
  printf 'Cannot locate Omen repository from %s\n' "$SCRIPT_DIR" >&2
  exit 1
fi

DEVICE_ID="${OMEN_DEVICE_ID:-}"
if [ -z "$DEVICE_ID" ]; then
  DEVICES=()
  while IFS= read -r serial; do
    DEVICES+=("$serial")
  done < <(adb devices | awk 'NR > 1 && $2 == "device" {print $1}')
  if [ "${#DEVICES[@]}" -ne 1 ]; then
    printf 'Set OMEN_DEVICE_ID; found %d online devices.\n' "${#DEVICES[@]}" >&2
    exit 1
  fi
  DEVICE_ID="${DEVICES[0]}"
fi

AFL_DIR="${OMEN_AFL_DIR:-$REPO_ROOT/assets/afl-binaries}"
python3 "$REPO_ROOT/.agents/skills/omen-pipeline/scripts/preflight.py" \
  --repo-root "$REPO_ROOT" \
  --stage fuzzing \
  --package "$PACKAGE" \
  --jni-name "$JNI_FUNC" \
  --device "$DEVICE_ID"

MAX_INPUT_SIZE="$(awk '/^[[:space:]]*#define[[:space:]]+BUFFER_SIZE[[:space:]]+[0-9]+$/ {print $3}' "$SCRIPT_DIR/fuzz.c")"
FRIDA_INPUT_SIZE="$(awk '/^[[:space:]]*#define[[:space:]]+BUF_LEN[[:space:]]+[0-9]+$/ {print $3}' "$AFL_SCRIPT")"
if [ -z "$MAX_INPUT_SIZE" ] || [ "$MAX_INPUT_SIZE" != "$FRIDA_INPUT_SIZE" ]; then
  printf 'BUFFER_SIZE and BUF_LEN must be explicit and equal.\n' >&2
  exit 1
fi

for required in "$BUILD_DIR/fuzz" "$AFL_SCRIPT" "$SCRIPT_DIR/seeds/seed" \
    "$PACKAGE_DIR/base.apk" "$AFL_DIR/afl-fuzz" "$AFL_DIR/afl-frida-trace.so"; do
  if [ ! -f "$required" ]; then
    printf 'Required file missing: %s\n' "$required" >&2
    exit 1
  fi
done

RESULT_DIR="$SCRIPT_DIR/results/$PROFILE"
mkdir -p "$RESULT_DIR" "$PACKAGE_DIR/logs"
RUN_LOG="$PACKAGE_DIR/logs/${JNI_FUNC}-${PROFILE}.log"
: >"$RUN_LOG"

LOCK_ACQUIRED=0
cleanup() {
  adb -s "$DEVICE_ID" shell "test -d $DEVICE_DIR/output/default && true" >/dev/null 2>&1 && \
    adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/fuzzer_stats" "$RESULT_DIR/fuzzer_stats" >>"$RUN_LOG" 2>&1 || true
  adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/crashes" "$RESULT_DIR/crashes" >>"$RUN_LOG" 2>&1 || true
  adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/hangs" "$RESULT_DIR/hangs" >>"$RUN_LOG" 2>&1 || true
  adb -s "$DEVICE_ID" shell "rm -rf $DEVICE_DIR" >>"$RUN_LOG" 2>&1 || true
  if [ "$LOCK_ACQUIRED" -eq 1 ]; then
    adb -s "$DEVICE_ID" shell "rmdir $LOCK_DIR" >>"$RUN_LOG" 2>&1 || true
  fi
}
trap cleanup EXIT

adb -s "$DEVICE_ID" wait-for-device
adb -s "$DEVICE_ID" shell "mkdir -p /data/local/tmp/omen && mkdir $LOCK_DIR" || {
  printf 'Another Omen fuzzing task holds the device slot.\n' >&2
  exit 1
}
LOCK_ACQUIRED=1
adb -s "$DEVICE_ID" shell "rm -rf $DEVICE_DIR && mkdir -p $DEVICE_DIR/seeds $DEVICE_DIR/output $DEVICE_DIR/lib"

adb -s "$DEVICE_ID" push "$BUILD_DIR/fuzz" "$DEVICE_DIR/fuzz" >>"$RUN_LOG" 2>&1
adb -s "$DEVICE_ID" push "$AFL_SCRIPT" "$DEVICE_DIR/afl.js" >>"$RUN_LOG" 2>&1
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/seeds/seed" "$DEVICE_DIR/seeds/seed" >>"$RUN_LOG" 2>&1
adb -s "$DEVICE_ID" push "$PACKAGE_DIR/base.apk" "$DEVICE_DIR/base.apk" >>"$RUN_LOG" 2>&1
adb -s "$DEVICE_ID" push "$AFL_DIR/afl-fuzz" "$DEVICE_DIR/afl-fuzz" >>"$RUN_LOG" 2>&1
adb -s "$DEVICE_ID" push "$AFL_DIR/afl-frida-trace.so" "$DEVICE_DIR/afl-frida-trace.so" >>"$RUN_LOG" 2>&1
for so in "$PACKAGE_DIR"/lib/arm64-v8a/*.so "$SCRIPT_DIR"/lib/*.so; do
  [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/" >>"$RUN_LOG" 2>&1
done
adb -s "$DEVICE_ID" shell "chmod +x $DEVICE_DIR/fuzz $DEVICE_DIR/afl-fuzz && chmod -R a+r $DEVICE_DIR/lib"

printf '[OMEN] release smoke, profile=%s, device=%s\n' "$PROFILE" "$DEVICE_ID" | tee -a "$RUN_LOG"
adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && timeout 30 env LD_LIBRARY_PATH=$DEVICE_DIR/lib ./fuzz < seeds/seed" >>"$RUN_LOG" 2>&1

printf '[OMEN] starting single AFL instance for %ss\n' "$DURATION" | tee -a "$RUN_LOG"
set +e
adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && timeout $DURATION env AFL_FRIDA_INST_NO_OPTIMIZE=1 AFL_FORKSRV_INIT_TMOUT=999999999 AFL_SKIP_CPUFREQ=1 AFL_NO_AFFINITY=1 ${OMEN_AFL_EXTRA_ENV:-} LD_LIBRARY_PATH=$DEVICE_DIR/lib ./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output ./fuzz" >>"$RUN_LOG" 2>&1
AFL_RC=$?
set -e
if [ "$AFL_RC" -ne 0 ] && [ "$AFL_RC" -ne 124 ]; then
  printf 'AFL exited with code %d; inspect %s\n' "$AFL_RC" "$RUN_LOG" >&2
  exit "$AFL_RC"
fi

adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/fuzzer_stats" "$RESULT_DIR/fuzzer_stats" >>"$RUN_LOG" 2>&1
printf 'Run complete. Metrics: %s/fuzzer_stats; log: %s\n' "$RESULT_DIR" "$RUN_LOG"
