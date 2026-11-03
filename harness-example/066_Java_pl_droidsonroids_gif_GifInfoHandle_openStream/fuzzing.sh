#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JNI_FUNC="$(basename "$SCRIPT_DIR")"
PROFILE="${OMEN_PROFILE:-release}"
DEVICE_ID="${OMEN_DEVICE_ID:?Set OMEN_DEVICE_ID to the target Android serial.}"
AFL_DIR="${OMEN_AFL_DIR:?Set OMEN_AFL_DIR to the directory containing afl-fuzz and afl-frida-trace.so.}"
DURATION="${OMEN_FUZZ_TIMEOUT:-240}"
DEVICE_DIR="/data/local/tmp/omen/examples/$JNI_FUNC"
LOCK_DIR="/data/local/tmp/omen/.examples.lock"

if [ "$PROFILE" = "debug" ]; then
  BUILD_DIR="$SCRIPT_DIR/build-debug"
  AFL_SCRIPT="$SCRIPT_DIR/afl-debug.js"
else
  PROFILE="release"
  BUILD_DIR="$SCRIPT_DIR/build"
  AFL_SCRIPT="$SCRIPT_DIR/afl.js"
fi

for required in "$BUILD_DIR/fuzz" "$AFL_SCRIPT" "$SCRIPT_DIR/seeds/seed" \
  "$AFL_DIR/afl-fuzz" "$AFL_DIR/afl-frida-trace.so"; do
  if [ ! -f "$required" ]; then
    printf 'Required file missing: %s\n' "$required" >&2
    exit 1
  fi
done

MAX_INPUT_SIZE="$(awk '/^[[:space:]]*#define[[:space:]]+BUFFER_SIZE[[:space:]]+[0-9]+$/ {print $3}' "$SCRIPT_DIR/fuzz.c")"
FRIDA_INPUT_SIZE="$(awk '/^[[:space:]]*#define[[:space:]]+BUF_LEN[[:space:]]+[0-9]+$/ {print $3}' "$AFL_SCRIPT")"
if [ -z "$MAX_INPUT_SIZE" ] || [ "$MAX_INPUT_SIZE" != "$FRIDA_INPUT_SIZE" ]; then
  printf 'BUFFER_SIZE and BUF_LEN must be explicit and equal.\n' >&2
  exit 1
fi

RESULT_DIR="$SCRIPT_DIR/results/$PROFILE"
mkdir -p "$RESULT_DIR"
LOCK_ACQUIRED=0
cleanup() {
  adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/fuzzer_stats" "$RESULT_DIR/fuzzer_stats" >/dev/null 2>&1 || true
  adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/crashes" "$RESULT_DIR/crashes" >/dev/null 2>&1 || true
  adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/hangs" "$RESULT_DIR/hangs" >/dev/null 2>&1 || true
  adb -s "$DEVICE_ID" shell "rm -rf $DEVICE_DIR" >/dev/null 2>&1 || true
  if [ "$LOCK_ACQUIRED" -eq 1 ]; then
    adb -s "$DEVICE_ID" shell "rmdir $LOCK_DIR" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

adb -s "$DEVICE_ID" wait-for-device
adb -s "$DEVICE_ID" shell "mkdir -p /data/local/tmp/omen && mkdir $LOCK_DIR" || {
  printf 'Another example fuzzing task holds the device slot.\n' >&2
  exit 1
}
LOCK_ACQUIRED=1
adb -s "$DEVICE_ID" shell "rm -rf $DEVICE_DIR && mkdir -p $DEVICE_DIR/seeds $DEVICE_DIR/output $DEVICE_DIR/lib"
adb -s "$DEVICE_ID" push "$BUILD_DIR/fuzz" "$DEVICE_DIR/fuzz" >/dev/null
adb -s "$DEVICE_ID" push "$AFL_SCRIPT" "$DEVICE_DIR/afl.js" >/dev/null
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/seeds/seed" "$DEVICE_DIR/seeds/seed" >/dev/null
adb -s "$DEVICE_ID" push "$AFL_DIR/afl-fuzz" "$DEVICE_DIR/afl-fuzz" >/dev/null
adb -s "$DEVICE_ID" push "$AFL_DIR/afl-frida-trace.so" "$DEVICE_DIR/afl-frida-trace.so" >/dev/null
for so in "$SCRIPT_DIR"/lib/*.so; do
  [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/" >/dev/null
done
if [ -n "${OMEN_BASE_APK:-}" ]; then
  adb -s "$DEVICE_ID" push "$OMEN_BASE_APK" "$DEVICE_DIR/base.apk" >/dev/null
fi
adb -s "$DEVICE_ID" shell "chmod +x $DEVICE_DIR/fuzz $DEVICE_DIR/afl-fuzz && chmod -R a+r $DEVICE_DIR/lib"

adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && timeout 30 env LD_LIBRARY_PATH=$DEVICE_DIR/lib ./fuzz < seeds/seed"
adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && timeout $DURATION env AFL_FRIDA_INST_NO_OPTIMIZE=1 AFL_FORKSRV_INIT_TMOUT=999999999 AFL_SKIP_CPUFREQ=1 AFL_NO_AFFINITY=1 ${OMEN_AFL_EXTRA_ENV:-} LD_LIBRARY_PATH=$DEVICE_DIR/lib ./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output ./fuzz"
