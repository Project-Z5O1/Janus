#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="emulator-5554"
JNI_FUNC="$(basename "$SCRIPT_DIR")"
DEVICE_DIR="/data/local/tmp/omen/$JNI_FUNC"

# Parallel fuzzing: OMEN_PARALLEL=N (0=auto, 1=single, N>1=parallel)
NUM_CORES=${OMEN_PARALLEL:-0}

adb -s "$DEVICE_ID" root
adb -s "$DEVICE_ID" wait-for-device
adb -s "$DEVICE_ID" shell "rm -rf $DEVICE_DIR"
adb -s "$DEVICE_ID" shell "mkdir -p $DEVICE_DIR/seeds $DEVICE_DIR/output $DEVICE_DIR/lib"

if [ ! -f "$SCRIPT_DIR/seeds/seed" ] && [ -f "$SCRIPT_DIR/seed" ]; then
  mkdir -p "$SCRIPT_DIR/seeds"
  cp "$SCRIPT_DIR/seed" "$SCRIPT_DIR/seeds/seed"
fi

adb -s "$DEVICE_ID" push "$SCRIPT_DIR/build/fuzz" $DEVICE_DIR/
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/afl.js" $DEVICE_DIR/
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/seeds/seed" $DEVICE_DIR/seeds/
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/../base.apk" $DEVICE_DIR/
for so in "$SCRIPT_DIR"/../lib/arm64-v8a/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done
for so in "$SCRIPT_DIR"/lib/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done

adb -s "$DEVICE_ID" push /Volumes/BIGSSD/Omen/assets/afl-binaries/afl-fuzz $DEVICE_DIR/
adb -s "$DEVICE_ID" push /Volumes/BIGSSD/Omen/assets/afl-binaries/afl-frida-trace.so $DEVICE_DIR/

adb -s "$DEVICE_ID" shell "chmod +x $DEVICE_DIR/fuzz $DEVICE_DIR/afl-fuzz"
adb -s "$DEVICE_ID" shell "chmod -R +r $DEVICE_DIR/lib"

# 从 harness 源码提取 BUFFER_SIZE 作为 -G 参数
MAX_INPUT_SIZE=$(grep '#define BUFFER_SIZE' "$SCRIPT_DIR/fuzz.c" | awk '{print $3}')
[ -z "$MAX_INPUT_SIZE" ] && MAX_INPUT_SIZE=256

# Auto-detect device cores if OMEN_PARALLEL=0 or unset
if [ "$NUM_CORES" -eq 0 ]; then
  NUM_CORES=$(adb -s "$DEVICE_ID" shell "nproc --all" | tr -d '\r\n')
fi
[ -z "$NUM_CORES" ] && NUM_CORES=1

echo "[OMEN] Parallel fuzzing: $NUM_CORES core(s), max_input=$MAX_INPUT_SIZE"

if [ "$NUM_CORES" -le 1 ]; then
  # ===== Single instance mode =====
  adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && \
    AFL_FRIDA_INST_NO_OPTIMIZE=1 \
    AFL_FORKSRV_INIT_TMOUT=999999999 \
    AFL_SKIP_CPUFREQ=1 \
    AFL_NO_AFFINITY=1 \
    LD_LIBRARY_PATH=$DEVICE_DIR/lib \
    ./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output ./fuzz"
else
  # ===== Multi-core parallel mode (Master + Slaves) =====
  STAGGER=20
  SLAVE_PIDS=()

  # Cleanup handler: kill local adb sessions → kills remote afl-fuzz
  cleanup() {
    echo "[OMEN] Stopping slave instances..."
    for pid in "${SLAVE_PIDS[@]}"; do
      kill "$pid" 2>/dev/null
    done
    wait "${SLAVE_PIDS[@]}" 2>/dev/null
    adb -s "$DEVICE_ID" shell "pkill -f 'afl-fuzz.*Slave' 2>/dev/null" || true
  }
  trap cleanup EXIT

  # Launch Slaves as local-background adb shell sessions.
  # Each Slave keeps a persistent adb connection (no broken pipe).
  # Device-side stdout/stderr → /dev/null so adb has no output to relay.
  for IDX in $(seq 2 "$NUM_CORES"); do
    echo "[OMEN] Launching Slave_$IDX ..."
    adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && \
      AFL_FRIDA_INST_NO_OPTIMIZE=1 \
      AFL_FORKSRV_INIT_TMOUT=999999999 \
      AFL_SKIP_CPUFREQ=1 \
      AFL_NO_AFFINITY=1 \
      LD_LIBRARY_PATH=$DEVICE_DIR/lib \
      ./afl-fuzz -O -G $MAX_INPUT_SIZE -i seeds -o output -S Slave_$IDX ./fuzz \
      >/dev/null 2>&1" &
    SLAVE_PIDS+=($!)
    echo "[OMEN] Slave_$IDX launched (pid $!), waiting ${STAGGER}s for initialization..."
    sleep "$STAGGER"
  done

  # Launch Master (foreground, shows AFL++ UI)
  echo "[OMEN] Launching Master (foreground)..."
  adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && \
    AFL_FRIDA_INST_NO_OPTIMIZE=1 \
    AFL_FORKSRV_INIT_TMOUT=999999999 \
    AFL_SKIP_CPUFREQ=1 \
    AFL_NO_AFFINITY=1 \
    LD_LIBRARY_PATH=$DEVICE_DIR/lib \
    ./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output -M Master ./fuzz" || true
fi
