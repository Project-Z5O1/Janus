#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="emulator-5554"
JNI_FUNC="$(basename "$SCRIPT_DIR")"
DEVICE_DIR="/data/local/tmp/omen/$JNI_FUNC"

adb -s "$DEVICE_ID" root
adb -s "$DEVICE_ID" wait-for-device
adb -s "$DEVICE_ID" shell "rm -rf $DEVICE_DIR"
adb -s "$DEVICE_ID" shell "mkdir -p $DEVICE_DIR/seeds $DEVICE_DIR/output $DEVICE_DIR/lib"

if [ ! -f "$SCRIPT_DIR/seeds/seed" ] && [ -f "$SCRIPT_DIR/seed" ]; then
  mkdir -p "$SCRIPT_DIR/seeds"
  cp "$SCRIPT_DIR/seed" "$SCRIPT_DIR/seeds/seed"
fi

adb -s "$DEVICE_ID" push "$SCRIPT_DIR/build-debug/fuzz" $DEVICE_DIR/fuzz-debug
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/afl-debug.js" $DEVICE_DIR/
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/seeds/seed" $DEVICE_DIR/seeds/
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/../base.apk" $DEVICE_DIR/
for so in "$SCRIPT_DIR"/../lib/arm64-v8a/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done
for so in "$SCRIPT_DIR"/lib/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done

adb -s "$DEVICE_ID" push /Volumes/BIGSSD/Omen/assets/afl-binaries/afl-fuzz $DEVICE_DIR/
adb -s "$DEVICE_ID" push /Volumes/BIGSSD/Omen/assets/afl-binaries/afl-frida-trace.so $DEVICE_DIR/

adb -s "$DEVICE_ID" shell "chmod +x $DEVICE_DIR/fuzz-debug $DEVICE_DIR/afl-fuzz"
adb -s "$DEVICE_ID" shell "chmod -R +r $DEVICE_DIR/lib"

# 从 harness 源码提取 BUFFER_SIZE 作为 -G 参数
MAX_INPUT_SIZE=$(grep '#define BUFFER_SIZE' "$SCRIPT_DIR/fuzz.c" | awk '{print $3}')
[ -z "$MAX_INPUT_SIZE" ] && MAX_INPUT_SIZE=256

adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && \
AFL_FRIDA_INST_NO_OPTIMIZE=1 \
AFL_FORKSRV_INIT_TMOUT=999999999 \
AFL_SKIP_CPUFREQ=1 \
AFL_NO_AFFINITY=1 \
AFL_DEBUG=1 \
AFL_FRIDA_JS_SCRIPT=$DEVICE_DIR/afl-debug.js \
OMEN_TRACE_STAGES=1 \
LD_LIBRARY_PATH=$DEVICE_DIR/lib \
./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output ./fuzz-debug"
