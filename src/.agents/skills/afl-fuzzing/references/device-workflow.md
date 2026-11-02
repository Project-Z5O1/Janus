# 真实设备执行与指标契约

本文给出 `afl-fuzzing` 的命令骨架。实际执行时替换 `<package>`、`<jni>`，并将所有路径、退出码
和日志保留为证据。命令中的仓库相对路径不是对工具或二进制存在性的承诺，preflight 必须验证。

## 1. 路径与工具解析

```bash
PACKAGE_DIR="target_APK/<package>"
WORK_DIR="$PACKAGE_DIR/<jni>"
DEVICE_DIR="/data/local/tmp/omen/<jni>"
REPO_ROOT="$(cd "$WORK_DIR/../../.." && pwd)"

AFL_FUZZ_BIN="${OMEN_AFL_FUZZ:-$REPO_ROOT/assets/afl-binaries/afl-fuzz}"
AFL_FRIDA_TRACE="${OMEN_AFL_FRIDA_TRACE:-$REPO_ROOT/assets/afl-binaries/afl-frida-trace.so}"
DEVICE_ID="${DEVICE_ID:-${ANDROID_SERIAL:-}}"
FUZZ_TIMEOUT="${OMEN_FUZZ_TIMEOUT:-240}"
```

- `OMEN_AFL_FUZZ` 和 `OMEN_AFL_FRIDA_TRACE` 用于显式配置宿主机上的 arm64 工具路径。
- 未配置时只检查上述仓库 `assets/` 候选位置；不存在就报 `DEPENDENCY`，不要声称它们已提供，
  也不要改成某台机器的绝对路径。
- `fuzzing.sh` 和 `fuzzing-debug.sh` 应以 `SCRIPT_DIR` 推导 `WORK_DIR` 与 `REPO_ROOT`，使用同样的
  环境变量约定，保证从目标目录直接执行时可复现。
- 构建工具链由 harness 阶段负责；本技能不生成或硬编码宿主机 NDK 路径。

## 2. 本地 preflight

### 2.1 必需文件

逐项执行存在性检查，并在错误中列出缺失的绝对解析结果或仓库相对路径：

```bash
test -x "$WORK_DIR/build/fuzz"
test -s "$WORK_DIR/afl.js"
test -d "$WORK_DIR/seeds"
test -s "$PACKAGE_DIR/base.apk"
test -s "$WORK_DIR/lib/libjenv.so"
test -s "$AFL_FUZZ_BIN"
test -s "$AFL_FRIDA_TRACE"
```

另需满足：

1. `seeds/` 至少一个非空普通文件；canonical smoke 种子优先为 `seeds/seed`，否则按确定顺序选取
   一个并记录文件名。
2. 根据 `jni_list.json` 和 `call_sequence.md` 列出目标及全部前置/factory 所在库，分别检查
   `$PACKAGE_DIR/lib/arm64-v8a/<lib>` 或 `$WORK_DIR/lib/<lib>` 有明确来源。
3. `afl.js` 白名单包含 `fuzz` 和实际目标库；若白名单还用于前置覆盖诊断，核对所有声明模块，
   但不得因此破坏目标 coverage reset。
4. debug 排障前再检查 `build-debug/fuzz`、`afl-debug.js`；其白名单使用上传后的可执行文件名
   `fuzz-debug`，并包含目标库。
5. 条件允许时检查 harness、目标库、AFL/Frida 文件架构均为设备支持的 arm64；工具版本必须兼容。
   无法检查架构是证据缺口，不能写成“已验证兼容”。

`build/fuzz`、`afl.js`、种子或尺寸配置缺失，且状态却声称 `harness=OK`，应返回
`FAIL/TEMPLATE_BUILD`，指出状态与产物不一致。APK、目标库、AFL/Frida 工具或架构环境缺失，
返回 `ENVIRONMENT/DEPENDENCY`。

### 2.2 输入尺寸一致性

从生成产物提取实际值并比较，禁止静默使用默认值：

```bash
MAX_INPUT_SIZE="$(awk '/^#define[[:space:]]+BUFFER_SIZE[[:space:]]+/ {print $3; exit}' \
  "$WORK_DIR/fuzz.c")"
test -n "$MAX_INPUT_SIZE"
case "$MAX_INPUT_SIZE" in (*[!0-9]*|'') exit 2;; esac
test "$MAX_INPUT_SIZE" -gt 0
```

继续检查：

- `afl.js` 与 `afl-debug.js` 的 `BUF_LEN`（或模板等价配置）等于 `MAX_INPUT_SIZE`。
- Algorithm 2 的最大布局需求不超过该值，解析器也不会读取超过该值。
- 验证命令、`fuzzing.sh` 和 `fuzzing-debug.sh` 的 `-G` 都直接使用这个已校验值。

缺值、非数字、零或不相等时停止。计划布局问题路由 `INPUT_CONSTRAINT`，生成器同步错误路由
`TEMPLATE_BUILD`；均不得通过任意 `-G 256` 继续。

## 3. 设备 preflight 与串行占用

设备选择规则：

1. `DEVICE_ID`/`ANDROID_SERIAL` 已给出时只使用该 serial。
2. 未给出时，仅当在线设备恰好一台才自动选择。
3. 零台或多台时分别报“无在线设备”或“必须唯一选择设备”，类别 `DEVICE`。

```bash
adb -s "$DEVICE_ID" wait-for-device
adb -s "$DEVICE_ID" root
adb -s "$DEVICE_ID" wait-for-device
adb -s "$DEVICE_ID" shell "id"
adb -s "$DEVICE_ID" shell "test -w /data/local/tmp"
adb -s "$DEVICE_ID" shell "getprop ro.product.cpu.abi"
```

若设备已经以 root 运行，`adb root` 的非破坏性结果可结合 `id` 判定；否则必须具有完成目录、
进程和日志操作所需的等效权限。不要反复重启 adbd 作为盲重试。

单设备串行要求：

- 启动前确认没有其他 Omen AFL 任务使用此设备；占用时返回 `ENVIRONMENT/DEVICE`。
- 不启动 Master/Slave，不启动多个 output 实例，不让两个 JNI 共用设备时间片。
- 不通过宽泛 `pkill` 清理不相关进程。需要终止时只针对本任务记录的 PID 和命令行。
- 本地脚本、设备 output 和日志均属于当前 JNI；禁止跨目标复用设备目录。

## 4. 部署 release

先创建本地日志目录，再仅清理当前 JNI 的设备目录：

```bash
mkdir -p "$WORK_DIR/logs"
LOG="$WORK_DIR/logs/fuzzing.log"
touch "$LOG"

adb -s "$DEVICE_ID" shell "rm -rf '$DEVICE_DIR' && \
  mkdir -p '$DEVICE_DIR/seeds' '$DEVICE_DIR/output' '$DEVICE_DIR/lib'"
adb -s "$DEVICE_ID" logcat -c
```

上传并校验：

```bash
adb -s "$DEVICE_ID" push "$WORK_DIR/build/fuzz" "$DEVICE_DIR/fuzz"
adb -s "$DEVICE_ID" push "$WORK_DIR/afl.js" "$DEVICE_DIR/afl.js"
adb -s "$DEVICE_ID" push "$WORK_DIR/seeds/." "$DEVICE_DIR/seeds/"
adb -s "$DEVICE_ID" push "$PACKAGE_DIR/base.apk" "$DEVICE_DIR/base.apk"
adb -s "$DEVICE_ID" push "$AFL_FUZZ_BIN" "$DEVICE_DIR/afl-fuzz"
adb -s "$DEVICE_ID" push "$AFL_FRIDA_TRACE" "$DEVICE_DIR/afl-frida-trace.so"

for so in "$PACKAGE_DIR"/lib/arm64-v8a/*.so "$WORK_DIR"/lib/*.so; do
  [ -f "$so" ] && adb -s "$DEVICE_ID" push "$so" "$DEVICE_DIR/lib/"
done

adb -s "$DEVICE_ID" shell "chmod +x '$DEVICE_DIR/fuzz' '$DEVICE_DIR/afl-fuzz' && \
  chmod -R a+r '$DEVICE_DIR/lib' '$DEVICE_DIR/seeds'"
adb -s "$DEVICE_ID" shell "test -x '$DEVICE_DIR/fuzz' && \
  test -s '$DEVICE_DIR/afl.js' && test -s '$DEVICE_DIR/base.apk' && \
  test -s '$DEVICE_DIR/afl-frida-trace.so' && \
  test -s '$DEVICE_DIR/lib/libjenv.so' && test ! -d '$DEVICE_DIR/lib/lib'"
```

上传后再次枚举远端 `seeds/` 和 `lib/`，逐项确认 preflight 生成的必需文件列表。包级与函数级
同名库冲突时不得静默覆盖；先根据共同输入确认应使用的版本，无法确认则停止并报告依赖冲突。

## 5. Release smoke

选择已记录的 smoke seed，直接执行 release：

```bash
adb -s "$DEVICE_ID" shell "cd '$DEVICE_DIR' && \
  LD_LIBRARY_PATH='$DEVICE_DIR/lib' timeout 30 ./fuzz < seeds/seed" \
  >>"$LOG" 2>&1
adb -s "$DEVICE_ID" shell "logcat -d -b crash -b main -b system" >>"$LOG" 2>&1
```

若 canonical 种子名称不同，命令必须使用实际选择的名称。保存 smoke 退出码；timeout、非零退出、
信号崩溃或日志显示目标未执行都不能进入 AFL。动态链接器错误是 `DEPENDENCY`；设备和权限错误是
`DEVICE`；参数/前置/清理问题按语义路由。

排障时可额外设 `OMEN_TRACE_STAGES=1`，但常规 release 首次运行先保持正常配置。完整日志必须落盘，
不得只截取最后几行作为主证据。

## 6. Release AFL 与 debug 后验

### 6.1 正式 release 验证

```bash
adb -s "$DEVICE_ID" shell "cd '$DEVICE_DIR' && \
  AFL_FRIDA_INST_NO_OPTIMIZE=1 \
  AFL_FORKSRV_INIT_TMOUT=999999999 \
  AFL_SKIP_CPUFREQ=1 \
  AFL_NO_AFFINITY=1 \
  LD_LIBRARY_PATH='$DEVICE_DIR/lib' \
  timeout '$FUZZ_TIMEOUT' ./afl-fuzz -O -D -G '$MAX_INPUT_SIZE' \
    -i seeds -o output ./fuzz" >>"$LOG" 2>&1
adb -s "$DEVICE_ID" shell "logcat -d -b crash -b main -b system" >>"$LOG" 2>&1
```

- 记录实际开始/结束时间、timeout 退出码和完整命令。timeout 到期是预设停止方式，不单独决定
  PASS/FAIL。
- `fuzzing.sh` 用于交互长跑时可不设置 timeout；若设置 `OMEN_FUZZ_TIMEOUT`，必须采用相同环境和
  参数，并在退出时完成日志采集。
- 若 AFL 在 forkserver 阶段退出，保存完整日志，不通过增加运行时长掩盖初始化失败。

### 6.2 Debug 复现

只有 release 失败后部署 debug：

```bash
adb -s "$DEVICE_ID" push "$WORK_DIR/build-debug/fuzz" "$DEVICE_DIR/fuzz-debug"
adb -s "$DEVICE_ID" push "$WORK_DIR/afl-debug.js" "$DEVICE_DIR/afl-debug.js"
adb -s "$DEVICE_ID" shell "chmod +x '$DEVICE_DIR/fuzz-debug'"
```

debug AFL 使用独立输出目录（例如 `output-debug`），并增加：

```bash
AFL_DEBUG=1
AFL_FRIDA_JS_SCRIPT="$DEVICE_DIR/afl-debug.js"
OMEN_TRACE_STAGES=1
```

目标命令为 `./fuzz-debug`，其余 `-G`、库路径和串行约束不变。debug 发现的问题修复后，必须清理并
重新部署 release，再完成 release smoke 和正式 AFL。

## 7. 指标采集与 hard gates

拉取当前运行的统计和发现物：

```bash
adb -s "$DEVICE_ID" pull "$DEVICE_DIR/output/default/fuzzer_stats" \
  "$WORK_DIR/fuzzer_stats"
adb -s "$DEVICE_ID" shell "cat '$DEVICE_DIR/output/default/fuzzer_stats'" >>"$LOG" 2>&1
```

若 crash/hang 目录存在，拉回 `$WORK_DIR/output/` 下对应发现物；设备可读时记录最新 tombstone
摘要。文件缺失或拉取失败必须在结果中说明。

`fuzzing_metrics.json` 至少包含：

```json
{
  "jni": "<jni>",
  "profile": "release",
  "run_seconds": 240,
  "forkserver_handshake": true,
  "bitmap_cvg": 0.38,
  "paths_total": 12,
  "execs_per_sec": 45.2,
  "stability": 91.0,
  "saved_crashes": 0,
  "saved_hangs": 0,
  "execs_done": 1000,
  "hard_gate_pass": true,
  "stats_path": "target_APK/<package>/<jni>/fuzzer_stats",
  "log_path": "target_APK/<package>/<jni>/logs/fuzzing.log",
  "artifact_fingerprints": {}
}
```

数值字段存数值，不把 `%` 字符串当数字比较；同时可在附加字段保留原始文本。`artifact_fingerprints`
至少能关联 `build/fuzz`、`afl.js`、关键库和种子，防止把旧统计归到新产物。

判定顺序：

1. 日志必须有明确 forkserver handshake 成功证据；AFL banner 或进程短暂存在不够。
2. `fuzzer_stats` 必须属于本次 `default` 实例，字段可解析。
3. `bitmap_cvg > 0`、`paths_total >= 2`、`execs_per_sec > 0` 全部成立。
4. `stability` 只记录，不作为否决条件。
5. 所有 hard gates 通过才可设置 `hard_gate_pass=true` 和 `fuzzing=OK`。

保存 crash/hang 并分析其是否来自目标或 harness；它们本身不替代 hard gates，也不自动使一个有效
fuzzing 运行失败。

## 8. 脚本、回流与清理

`fuzzing.sh` 和 `fuzzing-debug.sh` 必须包含各自实际的 preflight、部署、smoke、环境设置和运行命令，
不能只是打印说明。脚本应：

- 使用 `set -euo pipefail`，对预期的 AFL timeout 单独捕获退出码；
- 支持上述设备/AFL 路径环境变量，不包含宿主机专用绝对路径；
- 不使用多实例并行，不默认清理其他 JNI 目录；
- 在异常退出时保留日志，并避免留下当前任务的后台进程。

运行中需要修 harness 时，返回规范 `category` 和 `repair_owner`。允许负责阶段修改 harness，但旧
`harness=OK` 随即不能作为新产物的验收证据；必须重新通过完整 harness 验收后再运行 AFL。

最后先保存指标与发现物，再清理：

```bash
adb -s "$DEVICE_ID" shell "rm -rf '$DEVICE_DIR'"
```

确认当前任务没有残留 AFL 进程和设备槽占用。清理失败若是设备/权限问题，应附在结果中并归
`environment`，不能静默宣称环境已恢复。
