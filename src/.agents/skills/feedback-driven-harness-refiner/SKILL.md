---
name: feedback-driven-harness-refiner
description: 面向单个 Android JNI 的 Janus Phase 3 反馈驱动 harness 生成与修复；基于 call_sequence、argument_plan 和构造代码填充固定模板，完成构建、语义审查、独立运行及短覆盖验证，并按 repair_owner 回流失败。适用于生成、审查或修复 fuzz.c、afl.js、前置调用、参数构造和覆盖率问题。
---

# Feedback-Driven Harness Refiner

对单个 JNI 执行“生成/修复 → 验证 → 归因 → 回流”。目标不是得到一份仅能编译的
`fuzz.c`，而是得到语义符合真实调用路径、能独立运行且能产生目标覆盖证据的 harness。

## 边界与状态

- 工作目录为 `target_APK/<package>/<jni>/`，共享模板与固定文件只从仓库根目录的
  `assets/` 获取；不维护技能私有副本。
- 可以读取流水线状态判断资格，但不得直接写 `summary.json`，也不得返回文本替换指令。
- 返回结构化结果，状态字段为 `harness`；由 `omen-pipeline` helper 统一写回状态、错误和重试信息。
- 每次调用完成一轮有界工作。不得通过盲重试消耗预算；再次执行必须有新的修复或环境证据。
- 只负责当前 JNI。需要上游重分析或环境修复时，返回 `category` 与 `repair_owner`，不要越权伪修。

## 共同输入契约

必须读取并交叉校验：

| 输入 | 内容 | 用途 |
|---|---|---|
| `target_APK/<package>/<jni>/call_sequence.md` | Algorithm 1 | 前置调用、顺序、返回值数据流、对象/句柄生命周期 |
| `target_APK/<package>/<jni>/argument_plan.md` 及其包含或引用的构造代码 | Algorithm 2 | 参数终态标签、输入布局、类型/状态/长度约束、构造函数 |
| `target_APK/<package>/jni_list.json` | JNI 元数据 | 目标及前置函数签名、库名、相对偏移 |

按需读取 `src/java/`、`src/smali/`、`src/native/` 验证真实调用路径。若已有
`harness_review.md` 或反馈记录，先逐项核对上轮问题是否真正关闭。

输入缺失或相互矛盾时不得猜测：

- 调用链、前置函数或句柄工厂缺失，路由到 `call-sequence`。
- 参数类型、构造状态或输入约束缺失，路由到 `argument-construction`。
- 工具、设备或二进制依赖不可用，返回 `ENVIRONMENT`，不得伪造后续通过证据。
- `UNRESOLVED_HANDLE` 直接失败回流；禁止把 opaque handle 降级为整数 fuzz 输入。

## 不可变模板

固定生命周期为：

```text
init -> get_obj -> call_jni -> release
```

- `init`：初始化运行环境、加载库并解析所有目标/前置/工厂函数地址。
- `get_obj`：按输入计划构造参数，按 Algorithm 1 执行前置调用并连接返回值。
- `call_jni`：只执行目标 JNI 调用及其异常/阶段记录。
- `release`：按确定顺序释放 native 资源和 JNI local refs；局部引用严格逆创建顺序释放。

只能填充模板公开的 `GENERATOR` slot。不得改写固定控制流、信号处理、阶段追踪、异常辅助、
覆盖率快照设施或构建脚本来绕过失败。详细规范见
[template-contract.md](references/template-contract.md)。

## 工作流程

### 1. 生成或修复 harness

1. 根据三份共同输入建立“函数签名—参数来源—前置步骤—释放动作”对照表。
2. 复制固定资产；仅在 `GENERATOR` slot 生成 `fuzz.c`、`afl.js`、`afl-debug.js` 和链接配置。
3. 所有目标及前置函数地址必须采用 `dlopen` + `get_library_base` + relative offset；
   目标函数禁止 `dlsym`，也不得把反编译 image base 当作运行时偏移。
4. opaque handle 只能由已解析 factory 产生，并按计划传递/释放；不得从 fuzz buffer 伪造指针。
5. 有前置调用时，在首个前置调用前恢复 bitmap、最后一个前置调用后且目标调用前保存基线；
   无前置调用时不添加覆盖率重置。
6. 输入布局必须确定、边界安全且符合 Algorithm 2。`BUFFER_SIZE`、Frida 读取长度和启动脚本
   `-G` 必须来自同一尺寸约束；禁止静默回退到任意默认值。
7. 生成符合布局的 `seeds/`，至少包含一个可通过 standalone smoke 的种子。

### 2. 构建与链接门

使用 release profile 构建并确认 `build/fuzz` 存在、可执行，所有声明的链接库均已解析。

- 生成代码的语法、类型、slot 或链接声明错误：`TEMPLATE_BUILD` → `harness`。
- 编译器、工具链、目标库或运行依赖缺失：`DEPENDENCY` → `environment`，结果为 `ENVIRONMENT`。
- release 是首次验收及最终验收 profile；debug 只用于失败后的复现和证据收集，debug PASS
  不能替代 release PASS。

### 3. 语义审查门

对照 Algorithm 1、Algorithm 2、`jni_list.json` 和真实源码审查：

1. 调用路径保真度：前置调用完整、顺序正确、返回值流向正确。
2. 参数解析充分性：JNI 类型、常量和变长布局正确， fuzz 输入能驱动预期值域。
3. 前置条件满足度：对象状态、资源生命周期、opaque handle 工厂和参数约束成立。
4. 异常与清理：JNI 异常可观测，NULL/失败路径明确，local refs 逆序且无重复释放。
5. 覆盖潜力：目标库在白名单内，主要分支可由输入或已建立状态触达。
6. 模板一致性：固定骨架未变，coverage reset 位置和尺寸配置正确。

将逐项证据、结论、阻塞问题和 `repair_owner` 写入
`target_APK/<package>/<jni>/harness_review.md`。结论使用 `APPROVED`、
`NEEDS_REVISION` 或 `REJECTED`；只有无阻塞问题的 `APPROVED` 可进入验收。
审查发现 opaque handle 被当作整数 fuzz 时必须拒绝。

### 4. Standalone/runtime 门

在真实 Android 设备上使用 release 二进制、明确设备目录和 `LD_LIBRARY_PATH` 直接运行，
不启动 AFL。至少用正常种子执行，并用边界输入复验输入耗尽/空数组等路径；需要验证重复迭代时
使用同一 release 二进制重复运行。

通过要求：

- 动态链接、运行环境和所有函数地址初始化成功。
- 完整走过 `init -> get_obj -> call_jni -> release`，正常退出且无 harness 自身信号崩溃。
- 无未解释的 JNI 异常、越界读取、local ref 泄漏/重复释放或前置资源生命周期错误。
- 完整 stdout/stderr、必要的阶段 breadcrumb 与设备崩溃日志已保存。

失败后可用 debug profile 和阶段追踪定位；修复后必须重新回到 release 验证。

### 5. 短覆盖门

对同一批 release 产物执行以下任一验证：

- `afl-showmap`：使用与正式 fuzzing 一致的 Frida 配置和库环境，保存 map、元数据及日志；或
- 短 AFL run：确认 forkserver 启动，保存 `fuzzer_stats`、完整日志及非零覆盖/路径证据。

证据必须能归因到目标库/目标路径，而非只有 AFL banner、harness 自身执行或前置调用覆盖。
若只有非零 tuple 数但无法排除前置覆盖，应补充差分、目标命中或其他可审计证据。
允许使用明确声明的等价 coverage evidence，但必须对应当前 `fuzz.c`、二进制、脚本和库，
记录验证方式及产物哈希；历史不同版本或 standalone 运行不能替代覆盖门。

缺少设备、AFL/Frida 工具或运行依赖时返回 `ENVIRONMENT`，不得把“未执行”写成 PASS。
覆盖仅触及前置或极浅参数检查时，按
[feedback-routing.md](references/feedback-routing.md) 归因。

## 通过条件与输出

只有以下四项全部通过，才可返回 `harness=OK`：

| 门 | 必需证据 |
|---|---|
| compile/link | release `build/fuzz` 与完整构建日志 |
| semantic review | `harness_review.md` 为 `APPROVED` |
| standalone/runtime | release 正常退出及运行日志 |
| coverage | 当前产物的短覆盖证据，或明确声明且可审计的等价证据 |

主要产物包括 `fuzz.c`、`afl.js`、`afl-debug.js`、链接/构建配置、`build/fuzz`、`seeds/`、
`harness_review.md` 和覆盖/运行证据。产物齐全但不能补齐任一门证据时仍不是 OK。

返回示例（字段按实际结果填写）：

```json
{
  "harness": "OK|FAIL|ENVIRONMENT",
  "category": null,
  "repair_owner": null,
  "repair_applied": "本轮实际修复，无则为空",
  "evidence": {
    "review": "target_APK/<package>/<jni>/harness_review.md",
    "runtime": "运行日志路径",
    "coverage": "覆盖证据路径"
  },
  "consume_retry": false
}
```

`OK` 与 `ENVIRONMENT` 必须 `consume_retry=false`；可修复的有效 `FAIL` 可为 true，由调用方按预算处理。
`FAIL` 必须给出精确类别、证据和回流所有者；
具体路由与修复后的重验范围见 [feedback-routing.md](references/feedback-routing.md)。
