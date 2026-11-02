---
name: fuzzing-test
description: |
  Run AFL++ fuzzing test on Android device to validate eligible JNI harness functionality.
  Use when: (1) harness has been generated and compiled by /jni-harness,
  (2) user wants to run fuzzing test to verify harness works,
  (3) user mentions "fuzz test", "run fuzzing", "test harness", "verify harness", or "start fuzzing".
  Prerequisites: Complete pipeline: /jni-analysis -> /jni-callsequence -> /jni-harness (with build),
  and the target function must have `phase3_build == OK`.
---

# Fuzzing Test

Run AFL++ Frida Mode fuzzing tests on an Android device.

## Responsibility Boundaries

- `fuzzing-test` handles only **a single JNI** for Phase 4 verification.
- caller (pipeline orchestrator) decides retry policy and JNI switching.
- `fuzzing-test` must not rewrite Phase 3 error fields; it returns only the current JNI device-side result.
- `fuzzing-test` returns only device-side results and evidence; do not return a `summary.json` patch, `oldString/newString`,
  or any edit directive.
- `fuzzing-test` must not directly rewrite Phase 2/3/3.5/4 runtime state fields in `summary.json`; `phase4_fuzz`,
  retries, and error fields must be written back by the orchestrator through the summary helper.

## Prerequisites

Path Source Convention:

- shared binaries referenced by shell commands in this skill, come from the repository root by default `/Users/syc/Person/Work/Omen/assets/`
- do not reintroduce or maintain a second skill-local `assets/` binary copy

| Source | Required files |
|------|----------|
| `/jni-harness` | `build/fuzz`, `afl.js`, `seeds/seed`, `lib/*.so` |
| `/jni-analysis` | `base.apk` |

Additional files used for troubleshooting:
- `build-debug/fuzz`
- `afl-debug.js`
- `fuzzing-debug.sh`

## Phase 4 Profile Policy

- The first Phase 4 device verification must use the release profile: `build/fuzz` + `afl.js` + `fuzzing.sh`
- Only after release smoke run or release AFL verification fails, may switch to the debug profile:
  `build-debug/fuzz` + `afl-debug.js` + `fuzzing-debug.sh`
- When resuming a JNI with an existing `phase4_last_error`, first follow post-failure troubleshooting and prefer the debug profile to collect evidence.
- The debug profile is for locating the issue, not for replacing formal acceptance; after the fix is complete, the final pass decision should return to
  the release profile for reconfirmation

## Eligibility Gate

Run device-side fuzzing only for `summary.json` entries where `phase3_build == "OK"`.

If the function was Phase 2 filtered (`FILTERED_NON_JAVA_NATIVE` / `FILTERED_NO_JAVA_CALLSITE`), or
`phase3_build != "OK"`, do not start AFL; the upstream caller should directly mark `phase4_fuzz` as `SKIP`.

If `target_APK/{package}/phase2_targets.md` exists, the target function must also belong to this manifest.

## Device Directory Convention

Each JNI function uses an **isolated work directory** on the device, to avoid parallel conflicts:

```
DEVICE_DIR=/data/local/tmp/omen/{jni_func}
```

All `{device_dir}` in the following documentation refer to this path.

## Workflow

### 1. Device Preflight Gate (must pass first)

Run fail-fast checks first; do not enter AFL if they fail:

```bash
adb devices
adb -s {device_id} root
adb -s {device_id} wait-for-device
adb -s {device_id} shell "id"
adb -s {device_id} shell "test -w /data/local/tmp"
```

Pass criteria:
- device is online
- `adb root` available (at least usable for the current fuzz task)
- `/data/local/tmp` writable

### 2. Cleanly initialize the device directory

```bash
adb -s {device_id} shell "rm -rf {device_dir}"
adb -s {device_id} shell "mkdir -p {device_dir}/seeds {device_dir}/output {device_dir}/lib"
mkdir -p target_APK/{package}/logs
: > target_APK/{package}/logs/{jni_func}.log
adb -s {device_id} logcat -c
```

### 3. Upload Files (union upload, to avoid missing libraries)

#### 3.1 Upload Harness Files

```bash
if [ ! -f target_APK/{package}/{jni_func}/seeds/seed ] && [ -f target_APK/{package}/{jni_func}/seed ]; then
  mkdir -p target_APK/{package}/{jni_func}/seeds
  cp target_APK/{package}/{jni_func}/seed target_APK/{package}/{jni_func}/seeds/seed
fi
adb -s {device_id} push target_APK/{package}/{jni_func}/build/fuzz {device_dir}/
adb -s {device_id} push target_APK/{package}/{jni_func}/afl.js {device_dir}/
adb -s {device_id} push target_APK/{package}/{jni_func}/seeds/seed {device_dir}/seeds/
adb -s {device_id} shell "chmod +x {device_dir}/fuzz"
```

#### 3.2 Upload APK and Libraries (important: package-level + function-level lib must all be pushed)

```bash
adb -s {device_id} push target_APK/{package}/base.apk {device_dir}/
for so in target_APK/{package}/lib/arm64-v8a/*.so; do [ -f "$so" ] && adb -s {device_id} push "$so" {device_dir}/lib/; done
for so in target_APK/{package}/{jni_func}/lib/*.so; do [ -f "$so" ] && adb -s {device_id} push "$so" {device_dir}/lib/; done
adb -s {device_id} shell "chmod -R +r {device_dir}/lib"
```

Description: `{jni_func}/lib` usually contains `libjenv.so`, must be present together with package-level libraries.
All `.so` files must finally land at `{device_dir}/lib/*.so`; do not push the whole `lib/` directory to
`{device_dir}/`; otherwise it creates the nested directory `{device_dir}/lib/lib/*.so`.

Only after entering debug troubleshooting mode, upload additionally:

```bash
adb -s {device_id} push target_APK/{package}/{jni_func}/build-debug/fuzz {device_dir}/fuzz-debug
adb -s {device_id} push target_APK/{package}/{jni_func}/afl-debug.js {device_dir}/
adb -s {device_id} shell "chmod +x {device_dir}/fuzz-debug"
```

Requirement:
- `afl-debug.js` `MODULE_WHITELIST` must include `fuzz-debug`; otherwise the debug harness itself is excluded and the handshake fails.

#### 3.3 Upload AFL++ Binaries

```bash
adb -s {device_id} push /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-fuzz {device_dir}/
adb -s {device_id} push /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-frida-trace.so {device_dir}/
adb -s {device_id} shell "chmod +x {device_dir}/afl-fuzz {device_dir}/afl-frida-trace.so"
```

### 4. Pre-run hard validation (all must pass)

```bash
adb -s {device_id} shell "test -x {device_dir}/fuzz"
adb -s {device_id} shell "test -f {device_dir}/afl.js"
adb -s {device_id} shell "test -f {device_dir}/seeds/seed"
adb -s {device_id} shell "test -f {device_dir}/lib/libjenv.so"
adb -s {device_id} shell "test ! -d {device_dir}/lib/lib"
adb -s {device_id} shell "ls {device_dir}/lib/*.so"
```

### 5. Smoke Run (AFL precondition)

First verify that the harness can start independently, to avoid mistaking linker/runtime issues for AFL issues:

```bash
adb -s {device_id} shell "cd {device_dir} && printf 'AAAA' | env LD_LIBRARY_PATH={device_dir}/lib./fuzz"
```

If smoke fails, return FAIL directly, record the error log, do not enter AFL.

Requirement:
- Do not depend on `$ORIGIN/lib` as the only loading method; smoke run must explicitly use absolute `{device_dir}/lib`
- Do not use again `head`, `tail` truncate primary evidence
- complete smoke-run stdout/stderr must be appended to `target_APK/{package}/logs/{jni_func}.log`
- smoke after completion, immediately append one `adb -s {device_id} logcat -d -b crash -b main -b system`
- when diagnosing a failure, set additionally `OMEN_TRACE_STAGES=1`, so the harness writes internal `stage` breadcrumb to the log

### 6. Run AFL++ Verification (240 s)

Run release AFL first by default. The `-G` value must match the harness `BUFFER_SIZE` (computed by Phase 3 parameter analysis):

```bash
# extract BUFFER_SIZE from harness source
MAX_INPUT_SIZE=$(grep '#define BUFFER_SIZE' "$SCRIPT_DIR/fuzz.c" | awk '{print $3}')
[ -z "$MAX_INPUT_SIZE" ] && MAX_INPUT_SIZE=256

adb -s {device_id} shell "cd {device_dir} && \
  AFL_FRIDA_INST_NO_OPTIMIZE=1 \
  AFL_FORKSRV_INIT_TMOUT=999999999 \
  AFL_SKIP_CPUFREQ=1 \
  AFL_NO_AFFINITY=1 \
  LD_LIBRARY_PATH={device_dir}/lib \
  timeout 240./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output./fuzz"
```

Only in post-release-failure troubleshooting scenarios, execute debug AFL (`-G` also extracted from the harness):

```bash
adb -s {device_id} shell "cd {device_dir} && \
  AFL_FRIDA_INST_NO_OPTIMIZE=1 \
  AFL_FORKSRV_INIT_TMOUT=999999999 \
  AFL_SKIP_CPUFREQ=1 \
  AFL_NO_AFFINITY=1 \
  AFL_DEBUG=1 \
  AFL_FRIDA_JS_SCRIPT={device_dir}/afl-debug.js \
  OMEN_TRACE_STAGES=1 \
  LD_LIBRARY_PATH={device_dir}/lib \
  timeout 240./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output./fuzz-debug"
```

Requirement:
- complete AFL stdout/stderr must be appended verbatim to `target_APK/{package}/logs/{jni_func}.log`
- after running, must append again `adb -s {device_id} logcat -d -b crash -b main -b system`
- if the device is rooted and a tombstone exists, append the latest tombstone filename or content summary
- if exact crash location is needed, set additionally `OMEN_TRACE_STAGES=1`; do not enable for normal long fuzzing runs by default
- `phase4_last_error` returned to the orchestrator should be a short summary, for example
  `SIGSEGV stage=call_target pc=libfoo.so+0x1234`; put detailed evidence only in log files

### 7. Bounded Retries (budgeted uniformly by the orchestrator)

Do not blindly retry, retry only after fixing according to the current failure information:

- The first attempt always uses the release profile, do not start with debug
- The next troubleshooting attempt after a release failure should switch to the The debug profile to collect stronger crash evidence
- If debug has helped locate and fix the issue, the next acceptance run should switch back to the release profile; Do not let a debug pass replace a release pass
- on every failure, append stderr/stdout to `target_APK/{package}/logs/{jni_func}.log`
- return the latest error text to the caller on every failure
- If a fix was made, also return the latest fix action to the caller
- If failure evidence exists only in scrolling console output and was not written to disk, treat this execution as invalid, do not consume retry budget directly

`summary.json` in `phase4_retries` is incremented uniformly by the caller; default budget is at most 3 failures.

### 8. Pass Criteria

AFL 240 seconds after the run completes, parse device-side `fuzzer_stats` to get quantitative metrics:

```bash
adb -s {device_id} shell "cat {device_dir}/output/default/fuzzer_stats"
```

#### 8.1 Must Pass (Hard Gate)

| Metric | Requirement | Rationale |
|------|------|------|
| AFL banner | displayed normally | basic startup check |
| Forkserver handshake | success | basic runtime check |
| `bitmap_cvg` | > 0% | produces at least some coverage feedback |
| `paths_total` | >= 2 | prove bitmap grew during the run; found at least 1 new path = coverage expanded from the initial seed baseline |
| `execs_per_sec` | > 0 | completed at least one execution; JNI + Frida has high overhead, no high threshold is set |

> **JNI scenario note**: JNI calls pass through JVM (GC / JIT / thread scheduling), after Frida instrumentation
> stability is naturally low, execs_per_sec is also much lower than pure native fuzzing.therefore stability
> is recorded only as Soft Metrics, does not block the pass/fail decision.

#### 8.2 Quality Metrics (Soft Metrics, recorded but non-blocking)

Record the following metrics to `phase4_metrics` for analysis, but do not use them for pass/fail decisions:

| Metric | Recorded item | Purpose |
|------|--------|------|
| `bitmap_cvg` | actual coverage percentage | evaluate how much target code the harness can reach |
| `paths_total` | number of unique paths found | evaluate input parsing diversity |
| `saved_crashes` | number of crashes found | 240s verifypreliminary findings during verification |
| `saved_hangs` | number of hangs found | detect infinite loops/deadlocks |
| `execs_per_sec` | execution speed | evaluate harness efficiency |
| `stability` | stability percentage | evaluate determinism |

#### 8.3 Metric Collection Method

```bash
# collect fuzzer_stats
STATS=$(adb -s {device_id} shell "cat {device_dir}/output/default/fuzzer_stats 2>/dev/null")

# parse key fields
STABILITY=$(echo "$STATS" | grep "stability" | awk -F: '{print $2}' | tr -d ' %')
PATHS=$(echo "$STATS" | grep "paths_total" | awk -F: '{print $2}' | tr -d ' ')
BITMAP=$(echo "$STATS" | grep "bitmap_cvg" | awk -F: '{print $2}' | tr -d ' %')
EXECS=$(echo "$STATS" | grep "execs_per_sec" | awk -F: '{print $2}' | tr -d ' ')
CRASHES=$(echo "$STATS" | grep "saved_crashes" | awk -F: '{print $2}' | tr -d ' ')
HANGS=$(echo "$STATS" | grep "saved_hangs" | awk -F: '{print $2}' | tr -d ' ')
```

Metric collection failure (such as fuzzer_stats does not exist)is equivalent to Hard Gate failure.

#### 8.4 Phase 4 Output Fields

Structured results returned to the orchestrator must include:

```json
{
  "verdict": "PASS|FAIL",
  "hard_gate_pass": true,
  "phase4_metrics": {
    "stability": 95.5,
    "paths_total": 12,
    "bitmap_cvg": 0.38,
    "execs_per_sec": 45.2,
    "saved_crashes": 0,
    "saved_hangs": 0
  },
  "phase4_last_error": ""
}
```

### 9. Save One-Click Scripts

After successful verification, Generate `target_APK/{package}/{jni_func}/fuzzing.sh`:

Requirement:
- this is a host-side one-click launch script, not a snippet that runs only inside the device
- in the script `{device_id}` must be replaced with the actual serial of the current device (from `adb devices` or
  pipeline CONTEXT in `device_id` get), do not keep brace placeholders
- when the user is in `target_APK/{package}/{jni_func}/` directory and directly runs `./fuzzing.sh`, it must automatically complete
  `adb root`, clean the directory, push files, supply AFL++ binaries, set permissions, and start AFL
- each JNI function uses an isolated device directory `DEVICE_DIR` to avoid parallel conflicts
- **supports parallel fuzzing**:through `OMEN_PARALLEL=N` environment variable controls core count
  - `OMEN_PARALLEL` unset or `=0`:auto-detect all CPU cores on the device
  - `OMEN_PARALLEL=1`:single-instance mode (compatible with old behavior)
  - `OMEN_PARALLEL=N` (N>1):Master + (N-1) Slave parallel mode
- Slave instances run through local background `adb shell`; the Master instance runs in the foreground and shows the AFL++ UI.
- stagger each instance by 20 seconds during startup, avoid initialization I/O peak conflicts
- Master after exit, use trap EXIT automatically clean up all Slave processes

```bash
#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="{device_id}"
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
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/./base.apk" $DEVICE_DIR/
for so in "$SCRIPT_DIR"/./lib/arm64-v8a/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done
for so in "$SCRIPT_DIR"/lib/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done

adb -s "$DEVICE_ID" push /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-fuzz $DEVICE_DIR/
adb -s "$DEVICE_ID" push /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-frida-trace.so $DEVICE_DIR/

adb -s "$DEVICE_ID" shell "chmod +x $DEVICE_DIR/fuzz $DEVICE_DIR/afl-fuzz"
adb -s "$DEVICE_ID" shell "chmod -R +r $DEVICE_DIR/lib"

# Extract BUFFER_SIZE from harness source as the -G argument
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
   ./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output./fuzz"
else
  # ===== Multi-core parallel mode (Master + Slaves) =====
  STAGGER=20
  SLAVE_PIDS=()

  # Cleanup handler: kill local adb sessions -> kills remote afl-fuzz
  cleanup() {
    echo "[OMEN] Stopping slave instances.."
    for pid in "${SLAVE_PIDS[@]}"; do
      kill "$pid" 2>/dev/null
    done
    wait "${SLAVE_PIDS[@]}" 2>/dev/null
    adb -s "$DEVICE_ID" shell "pkill -f 'afl-fuzz.*Slave' 2>/dev/null" || true
  }
  trap cleanup EXIT

  # Launch Slaves as local-background adb shell sessions.
  # Each Slave keeps a persistent adb connection (no broken pipe).
  # Device-side stdout/stderr -> /dev/null so adb has no output to relay.
  for IDX in $(seq 2 "$NUM_CORES"); do
    echo "[OMEN] Launching Slave_$IDX.."
    adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && \
      AFL_FRIDA_INST_NO_OPTIMIZE=1 \
      AFL_FORKSRV_INIT_TMOUT=999999999 \
      AFL_SKIP_CPUFREQ=1 \
      AFL_NO_AFFINITY=1 \
      LD_LIBRARY_PATH=$DEVICE_DIR/lib \
     ./afl-fuzz -O -G $MAX_INPUT_SIZE -i seeds -o output -S Slave_$IDX./fuzz \
      >/dev/null 2>&1" &
    SLAVE_PIDS+=($!)
    echo "[OMEN] Slave_$IDX launched (pid $!), waiting ${STAGGER}s for initialization.."
    sleep "$STAGGER"
  done

  # Launch Master (foreground, shows AFL++ UI)
  echo "[OMEN] Launching Master (foreground).."
  adb -s "$DEVICE_ID" shell "cd $DEVICE_DIR && \
    AFL_FRIDA_INST_NO_OPTIMIZE=1 \
    AFL_FORKSRV_INIT_TMOUT=999999999 \
    AFL_SKIP_CPUFREQ=1 \
    AFL_NO_AFFINITY=1 \
    LD_LIBRARY_PATH=$DEVICE_DIR/lib \
   ./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output -M Master./fuzz" || true
fi
```

Also generate `target_APK/{package}/{jni_func}/fuzzing-debug.sh`, for troubleshooting (always single-instance):

```bash
#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="{device_id}"
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
adb -s "$DEVICE_ID" push "$SCRIPT_DIR/./base.apk" $DEVICE_DIR/
for so in "$SCRIPT_DIR"/./lib/arm64-v8a/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done
for so in "$SCRIPT_DIR"/lib/*.so; do [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"; done

adb -s "$DEVICE_ID" push /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-fuzz $DEVICE_DIR/
adb -s "$DEVICE_ID" push /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-frida-trace.so $DEVICE_DIR/

adb -s "$DEVICE_ID" shell "chmod +x $DEVICE_DIR/fuzz-debug $DEVICE_DIR/afl-fuzz"
adb -s "$DEVICE_ID" shell "chmod -R +r $DEVICE_DIR/lib"

# Extract BUFFER_SIZE from harness source as the -G argument
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
./afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output./fuzz-debug"
```

### 10. Clean Up Environment (whether success or failure)

```bash
adb -s {device_id} shell "rm -rf {device_dir}"
```

## Phase 4 Terminal State Convention

For a single JNI, `fuzzing-test` output can only be one of the following results:

- `PASS`: smoke run and AFL verification pass (with all Hard Gate metrics passing)
- `FAIL`: smoke run or AFL verification failure, or Hard Gate metrics do not pass
- `SKIP`: upstream `phase3_build != OK` or `phase3_review != APPROVED`, do not start device verification

caller must be responsible for writing results back:

- `phase4_retries`
- `phase4_last_error`
- `phase4_fix_applied`
- `phase4_metrics` (structured metrics object)

and terminalize the current JNI after the retry budget is reached.
if you start constructing `summary.json` partial text replacement, the current caller has bypassed the summary helper.


## AFL++ Environment Variables

| Variable | Description |
|------|------|
| `AFL_FRIDA_INST_NO_OPTIMIZE=1` | disable Frida optimization, improve stability |
| `AFL_FORKSRV_INIT_TMOUT=999999999` | extend fork server initialization timeout |
| `AFL_SKIP_CPUFREQ=1` | skip CPU frequency checks |
| `AFL_DEBUG=1` | enable debug output (for testing) |

## afl-fuzz Arguments

| Arguments | Description |
|------|------|
| `-O` | use Frida mode |
| `-D` | enable deterministic mutations |
| `-G <MAX_INPUT_SIZE>` | Maximum input size; must match harness `BUFFER_SIZE` (calculated by Phase 3) |
| `-i seeds` | input seed directory |
| `-o output` | output directory |

## FAQ

| Issue | Solution |
|------|----------|
| `afl-fuzz: command not found` | ensure it has been uploaded and chmod +x |
| `Frida RTLD not found` | ensure afl-frida-trace.so is in the same directory |
| `No instrumentation detected` | check afl.js MODULE_WHITELIST |
| `Fork server handshake failed` | first troubleshoot preceding linker issues; try if necessary `AFL_MAP_SIZE=10000000` |
| `shmget() failed` / `Permission denied` | execute `adb root && adb wait-for-device` then retry |
| `library "libjenv.so" not found` | ensure `target_APK/{package}/{jni_func}/lib` has been pushed to `{device_dir}/lib` and `LD_LIBRARY_PATH` correct |

## Assets

| File | Description |
|------|------|
| `/Users/syc/Person/Work/Omen/assets/afl-binaries/afl-fuzz` | AFL++ fuzzer (arm64) |
| `/Users/syc/Person/Work/Omen/assets/afl-binaries/afl-frida-trace.so` | Frida mode library |
