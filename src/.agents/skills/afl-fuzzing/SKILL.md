---
name: afl-fuzzing
description: 在单台真实 Android 设备上对 harness==OK 的 JNI 目标串行执行 AFL++ Frida Mode 验证；完成 preflight、release/debug 脚本、设备部署、smoke、fuzzer_stats 指标和 hard gates 判定，并将 harness 语义问题按 repair_owner 回流。适用于启动设备 fuzzing、检查 AFL 覆盖指标或诊断 forkserver/覆盖失败。
---

# AFL Fuzzing

只对已经完整通过 harness 验收的单个 JNI 运行真实设备 AFL++ Frida Mode fuzzing。
本技能产出可复现启动脚本和量化指标，不用静态推断或模拟输出代替设备执行。

## 边界与状态

- 工作目录为 `target_APK/<package>/<jni>/`，设备目录固定为
  `/data/local/tmp/omen/<jni>`。
- 资格门是 `harness == "OK"`；否则不启动 AFL，返回 `SKIP` 或由上游先完成 harness 修复。
- 单设备、单目标、单实例串行执行。不得在同一设备并行启动多个 Omen fuzzing worker，也不以
  Master/Slave 多实例绕过串行约束；发现设备槽被占用时返回 `ENVIRONMENT/DEVICE`。
- 可以读取状态，但不得直接写 `summary.json`。返回状态字段为 `fuzzing`，由
  `omen-pipeline` helper 写回状态、指标、错误与重试信息。
- 环境失败返回 `ENVIRONMENT` 且 `consume_retry=false`，不得消耗无效模型重试；代码/语义失败
  必须先修复再重试。

## 输入与共同契约

除已验收 harness 产物外，读取共同输入用于依赖、尺寸与运行问题归因：

- `target_APK/<package>/<jni>/call_sequence.md`（Algorithm 1）：目标及前置函数、库和生命周期。
- `target_APK/<package>/<jni>/argument_plan.md`（Algorithm 2）及其构造代码：输入布局、长度和
  状态约束。
- `target_APK/<package>/jni_list.json`：目标/前置函数签名、目标库与相对偏移。

这些输入与当前 `fuzz.c` 不一致时，不得通过修改 AFL 参数掩盖问题；按 `repair_owner` 回流。

## Preflight：任何缺失都必须明确报错

在清理设备目录或启动 AFL 前完成本地与设备检查。详细命令见
[device-workflow.md](references/device-workflow.md)。

### 本地产物

- `build/fuzz` 存在且可执行，`afl.js` 存在并与 release 二进制对应。
- `seeds/` 至少有一个非空、符合 Algorithm 2 布局的种子；不要静默用任意字节替代缺失种子。
- `target_APK/<package>/base.apk` 存在。
- `target_APK/<package>/lib/arm64-v8a/` 中目标及前置依赖库、当前 JNI `lib/` 中的运行库
  （含 `libjenv.so`）均存在；上传后必须平铺到设备 `lib/*.so`。
- AFL++ Frida Mode 所需的 `afl-fuzz` 和 `afl-frida-trace.so` 可定位、架构与设备匹配且版本兼容。
  路径可由环境配置指定，也可检查仓库根 `assets/` 候选位置；不得假设文件必然存在。
- `fuzz.c` 的 `BUFFER_SIZE`、release/debug Frida 配置读取长度和 `-G` 一致。无法提取或值不一致
  时停止，不得默认成 256。

### 设备与串行槽

- 唯一确定设备 serial；未指定且存在多台设备时明确报 `DEVICE`，不要随机选择。
- 设备在线、ABI 匹配，`adb root` 可用或已具备等效权限，`/data/local/tmp` 可写。
- 当前设备没有其他 Omen AFL 任务占用；不要为抢占设备而杀死不相关进程。
- release 首次运行所需文件齐全。debug 文件只在失败排障时追加检查。

本地 harness 必需产物缺失或与 `harness=OK` 状态矛盾时，返回 `FAIL/TEMPLATE_BUILD` 并回流
`harness`；设备、工具、APK 或共享库依赖不可用则返回 `ENVIRONMENT` 和具体缺失项。

## 执行流程

### 1. 生成可复现脚本

从仓库根 `assets/` 原样复制固定启动脚本，不重新手写部署逻辑：

- `assets/fuzzing.sh` → `fuzzing.sh`：release `build/fuzz` + `afl.js`，完成 preflight、设备单槽、干净部署、smoke 和单实例 AFL。
- `assets/fuzzing-debug.sh` → `fuzzing-debug.sh`：通过 `OMEN_PROFILE=debug` 调用同一入口，使用 debug 二进制与 Frida 配置。

脚本从自身位置推导包目录和仓库根目录，设备 serial 与 AFL 工具路径支持环境配置。
不得写入某台宿主机的 NDK/AFL 绝对路径，也不得声称共享二进制必然随仓库提供。
`-G` 必须读取并与 harness 尺寸校验，不得静默回退。

### 2. 干净部署

1. 仅清理当前 JNI 的 `/data/local/tmp/omen/<jni>`，创建 `seeds/`、`output/`、`lib/`。
2. 上传 release harness、`afl.js`、种子、`base.apk`、包级和函数级 `.so`，以及 AFL/Frida 文件。
3. 设置最小必要权限，并远端确认 `fuzz` 可执行、种子/APK/脚本存在、所有库直接位于
   `DEVICE_DIR/lib/`；出现 `lib/lib/` 嵌套即为部署错误。
4. 本地日志使用追加写，保存完整 stdout/stderr，不截断主证据。

### 3. Release smoke

先在设备直接运行 release `fuzz`，显式设置 `LD_LIBRARY_PATH="$DEVICE_DIR/lib"`，使用已部署
种子并设置有界超时。保存退出码、完整输出和 crash/main/system 日志。

smoke 未正常退出时不进入 AFL：链接/设备问题归环境；参数、前置调用、目标状态或 release 清理
问题按反馈契约回流。不得用跳过目标调用的方式获得 smoke PASS。

### 4. 真实 AFL 验证

使用 release profile 和单个 `default` 实例运行有界 AFL，默认验证时长 240 秒，可由本次流水线
配置调整并记录。保持 `-O` Frida 模式，`-G` 等于已验证的 `BUFFER_SIZE`，并设置 JNI/Frida
所需的 forkserver 超时、CPU 检查跳过和库路径环境。

运行结束后保存：

- 完整 AFL stdout/stderr 与设备日志；
- `output/default/fuzzer_stats`；
- crash/hang 输入及必要的 tombstone 摘要；
- 本次二进制、脚本、库与参数的标识信息。

超时终止本身不等于失败，应以 forkserver 证据和最终指标判定；但无法取得完整日志或
`fuzzer_stats` 时不能判 OK。

### 5. Debug 后验

只有 release smoke/AFL 失败后，才使用 `fuzzing-debug.sh` 复现并收集更详细证据。
debug 用于定位，不替代正式验收；修复后必须重新执行 release smoke 和 release AFL。

## Hard Gates 与 Soft Metrics

只有真实 release 运行同时满足以下全部条件，才返回 `fuzzing=OK`：

| Hard gate | 通过条件 | 证据 |
|---|---|---|
| Forkserver handshake | 明确成功 | AFL 完整日志中的握手成功证据，不能仅以进程存在代替 |
| `bitmap_cvg` | `> 0%` | 当前运行的 `fuzzer_stats` |
| `paths_total` | `>= 2` | 当前运行的 `fuzzer_stats`，证明覆盖从初始基线增长 |
| `execs_per_sec` | `> 0` | 当前运行的 `fuzzer_stats`，证明至少完成有效执行 |

任一指标缺失、不可解析或不达标即不能 OK。`stability` 只作 Soft Metric；JNI/JVM/Frida
开销导致其偏低时，不单独阻塞通过。同步记录 `saved_crashes`、`saved_hangs`、`execs_done`、
运行时长和实际 `bitmap_cvg`/`paths_total`/`execs_per_sec`。crash 是测试结果，不因发现 crash
自动判 harness 失败，但必须保存样本并防止与 harness 自身崩溃混淆。

将指标写入 `target_APK/<package>/<jni>/fuzzing_metrics.json`，包含 hard gate 布尔值、原始指标、
profile、日志/统计文件路径和产物标识。返回结果中附同一指标对象或其路径。

## 运行中反馈与修复回流

发现 harness 语义问题时，不禁止修复 harness；但必须通过 `repair_owner` 回到负责阶段，
修复后重新完成 harness 的 compile、semantic review、runtime、coverage 四门，再恢复 AFL。
不得由 fuzzing 执行者静默改完 harness 后直接沿用旧 `harness=OK`。

| `category` | `repair_owner` |
|---|---|
| `TEMPLATE_BUILD` | `harness` |
| `MISSING_PREREQUISITE` / `INVALID_CALL_ORDER` / `UNRESOLVED_HANDLE` | `call-sequence` |
| `ARGUMENT_TYPE` / `ARGUMENT_STATE` / `INPUT_CONSTRAINT` / `SHALLOW_COVERAGE` | `argument-construction` |
| `DEVICE` / `DEPENDENCY` | `environment` |

覆盖增长不足时先排除白名单、前置覆盖污染、前置缺失和输入约束问题，再归 `SHALLOW_COVERAGE`。
环境失败不得通过反复调用模型或修改 harness 来“重试”；只有环境恢复或产物发生变化后才重跑。

## 返回契约与清理

成功结果示例：

```json
{
  "fuzzing": "OK",
  "category": null,
  "repair_owner": null,
  "metrics": "target_APK/<package>/<jni>/fuzzing_metrics.json",
  "hard_gate_pass": true,
  "consume_retry": false
}
```

失败使用 `fuzzing=FAIL` 并给出类别、责任方和完整证据；环境不可用使用
`fuzzing=ENVIRONMENT`、`consume_retry=false`。资格不满足使用 `SKIP`，不启动设备任务。

清理前先拉回指标、日志和 crash/hang 样本；随后只删除当前 JNI 设备目录。无论成功或失败，
都不得留下后台 AFL 进程占用单设备串行槽。
