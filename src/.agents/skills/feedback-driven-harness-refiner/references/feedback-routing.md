# 失败归因与反馈路由

反馈的目的，是让下一次执行修复导致失败的产物，而不是让同一 worker 反复尝试相同输入。
本路由同样适用于短覆盖验证和正式 AFL 运行中暴露的 harness 语义问题。

## 1. 统一反馈记录

每次 `FAIL` 或 `ENVIRONMENT` 至少返回：

```json
{
  "status": "FAIL|ENVIRONMENT",
  "category": "TEMPLATE_BUILD",
  "repair_owner": "harness",
  "failed_gate": "compile|review|runtime|coverage|preflight|smoke|fuzzing",
  "evidence": ["完整日志或报告路径"],
  "observed": "实际现象的短摘要",
  "expected": "被违反的契约",
  "repair_applied": "本轮已经实施的修复，无则为空",
  "consume_retry": true
}
```

要求：

- `evidence` 指向完整日志、map、`fuzzer_stats` 或审查报告，不能只保留控制台摘要。
- `observed` 与 `expected` 应能定位到函数、参数、step、输入字段或依赖。
- `repair_owner` 是下一次修复的责任阶段，不是当前执行者自行选择的便利路径。
- `ENVIRONMENT` 必须 `consume_retry=false`；环境恢复后重新执行原门，不消耗无效模型重试。
- 没有新修复、没有新环境状态或没有新证据时，不得发起盲目重试。

## 2. 规范路由表

| `category` | `repair_owner` | 典型证据 | 必须修复的产物 |
|---|---|---|---|
| `TEMPLATE_BUILD` | `harness` | GENERATOR 代码语法/类型错误、签名与生成 typedef 不符、模板固定区被改、白名单/链接声明由生成错误导致 | 当前 JNI 的 harness slot 与生成配置 |
| `MISSING_PREREQUISITE` | `call-sequence` | Algorithm 1 缺少初始化/writer/factory，目标状态来源无前置步骤 | `call_sequence.md` 的调用闭包与前置表 |
| `INVALID_CALL_ORDER` | `call-sequence` | 前置步骤错序、返回值在生成前使用、释放早于最后使用、真实调用路径与计划冲突 | 前置步骤顺序、数据流和资源生命周期 |
| `UNRESOLVED_HANDLE` | `call-sequence` | opaque handle 无 factory、句柄证据未被 Algorithm 1 识别、factory 元数据不闭合 | 句柄聚类、factory 及其库/偏移/返回值用途 |
| `ARGUMENT_TYPE` | `argument-construction` | JNI 类型宽度/符号错误、数组元素类型错误、handle 已有 factory 但构造代码仍读整数、构造返回值类型不符 | `argument_plan.md` 和对应构造代码 |
| `ARGUMENT_STATE` | `argument-construction` | 对象已分配但未初始化、字段/writer 状态不满足、factory 失败策略错误、资源状态与目标要求不符 | 对象状态构造、字段设置和有效/无效状态策略 |
| `INPUT_CONSTRAINT` | `argument-construction` | 长度前缀/游标错误、越界读、数组分配失控、字符串编码非法、尺寸三处不一致源于布局计划、种子不符合布局 | 输入布局、clamping、长度上限和种子约束 |
| `SHALLOW_COVERAGE` | `argument-construction` | 仅触及参数检查/前置调用，目标分支由不可变值或未建立状态阻断，短跑无目标路径增长 | 可变异字段、值域、状态组合和覆盖导向种子 |
| `DEVICE` | `environment` | 无设备、多设备但未唯一选择、设备掉线、目录不可写、设备槽被占用、ABI 不兼容 | 设备连接、权限、串行槽或设备配置 |
| `DEPENDENCY` | `environment` | 编译/运行工具缺失、APK/目标库/AFL/Frida 二进制缺失、动态链接依赖不可用、工具版本不兼容 | 工具链、依赖文件和运行环境 |

## 3. 归因规则

### 构建与链接

- 先看是否是工具/文件不存在，再看生成代码错误。缺少编译器、SDK/NDK 工具或 `.so` 是
  `DEPENDENCY`，不能靠修改 `fuzz.c` 假装解决。
- 编译器指出当前 GENERATOR slot 的语法、函数指针类型或 JNI 类型错误，是 `TEMPLATE_BUILD`；
  若根本原因是 Algorithm 2 给错参数类型，则主类别为 `ARGUMENT_TYPE`。
- 链接声明漏库但共同输入中的库信息正确，是 `TEMPLATE_BUILD`；库文件本身缺失是 `DEPENDENCY`。

### 调用路径与参数

- “不知道要调用什么”归 `call-sequence`；“已知要构造什么但构造错”归
  `argument-construction`；“把已知计划填错 slot”归 `harness`。
- 前置函数缺失和顺序错误不得在 harness 中凭空补猜。只有 Algorithm 1 已明确、生成器漏填时，
  才以 `TEMPLATE_BUILD` 修复当前 harness。
- opaque handle 若没有 factory，统一 `UNRESOLVED_HANDLE`；factory 已明确而生成/构造代码仍
  从 buffer 读整数，按实际错误产物归 `TEMPLATE_BUILD` 或 `ARGUMENT_TYPE`。两种情况下都禁止
  把句柄降级为普通整数输入。
- 目标只在某种对象状态下有效，但计划没有状态构造方法，是 `ARGUMENT_STATE`；缺少建立该状态
  所需的 native 前置调用，则优先 `MISSING_PREREQUISITE`。

### Runtime 与 standalone

- 设备不可用、loader 找不到依赖、权限不足归环境，不消耗模型重试。
- 正常 seed 在 `get_obj` 崩溃，优先检查输入边界和对象状态；在特定前置 step 崩溃，核对该 step
  的参数及顺序；在 `release` 崩溃，检查资源对称性和 local ref 逆序。
- 目标自身在合法输入下崩溃可能是 fuzzing 发现，不应自动归为 harness 缺陷。先用 debug、阶段
  breadcrumb、真实调用约束和最小复现区分目标缺陷与错误调用；未归因前不得判 harness OK。
- 通过删除目标调用、跳过异常、吞掉信号或取消 release 获得的“PASS”无效。

### Coverage

- 目标库未进入 Frida 白名单、入口/地址配置错误导致没有目标插桩，是 `TEMPLATE_BUILD`。
- 前置缺失导致目标总提前返回，是 `MISSING_PREREQUISITE`；前置顺序错是 `INVALID_CALL_ORDER`。
- 目标和前置均有执行证据，但输入只能触发浅层参数检查，是 `SHALLOW_COVERAGE`，回流参数构造，
  增加有意义的输入维度或状态组合，而不是单纯延长 fuzz 时间。
- 非零 bitmap 不能自动证明目标覆盖。先用 coverage reset、差分或目标命中日志排除 prereq-only
  覆盖，再决定是否为浅覆盖。
- 缺少覆盖工具或设备时返回 `ENVIRONMENT`，不得以静态“理论可覆盖”代替动态证据。

## 4. 修复后的重验范围

| 回流所有者 | 修复后必须重新执行 |
|---|---|
| `harness` | compile/link → semantic review → standalone/runtime → coverage |
| `call-sequence` | 校验并更新 Algorithm 2 的一致性，再重新生成/修复 harness，完整四门重验 |
| `argument-construction` | 重新生成受影响构造代码和种子，再完整四门重验 |
| `environment` | 从被环境阻断的门继续；若任何二进制、库或脚本改变，则重跑其后的全部门 |

任何影响 `fuzz.c`、构建产物、Frida 脚本、目标库或输入布局的修复，都会使旧 coverage evidence
失效。正式 AFL 中回流修复 harness 后，也必须先重新取得 `harness=OK`，再恢复 fuzzing。

## 5. 路由闭合

一次反馈只有满足以下条件才算关闭：

1. `repair_owner` 接受的修复有具体变更记录，而非仅重写错误描述。
2. 新证据对应修复后的产物，且能直接反驳原 `observed`。
3. 所有受影响的后续门重新通过，未沿用旧日志或旧 map。
4. `harness_review.md` 更新原发现并说明剩余 WARN；没有隐藏的阻塞项。
5. 环境失败已恢复时，明确记录恢复项；不把环境恢复算作代码修复。
