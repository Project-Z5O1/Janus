---
name: dependency-driven-call-sequence
description: 从 Java 与 native 证据提取单个 JNI 的依赖闭包、前置调用序列和参数来源；当需要恢复 native 初始化、句柄工厂、字段写回、返回值数据流或生成 call_sequence.md 时使用。
---

# Algorithm 1: Dependency-Driven Call Sequence Extraction

## 目标与边界

对单个目标 JNI 方法 `m`，从当前未满足依赖出发，迭代恢复应用真实可达的前置调用序列。
不要从调用者树一次性猜完整链路，也不要求追踪到 Activity/Service 入口。

本技能只负责：

- 前置 Java/JNI 调用及其顺序；
- 参数来源、显式/隐式依赖和跨语言证据；
- 交给 `recursive-argument-constructor` 的复杂对象构造需求。

本技能**不生成**复杂对象构造代码、recipe、完整 harness，也不直接修改 `summary.json`。

- 状态阶段名：`call-sequence`
- 工作目录：`target_APK/<package>/<jni>/`
- 主输出：`target_APK/<package>/<jni>/call_sequence.md`

## 输入与前置条件

输入为论文 Algorithm 1 的 `m / Γ / T`：

- `m`：目标 JNI 的完整 Java 签名、JNI 名、library、offset；
- `Γ`：`target_APK/<package>/src/` 下 Java、Smali、Ghidra native 代码及 `jni_list.json`；
- `T`：`structure-aware-retriever` 与 `harness-memory` 提供的检索和复用能力。

先确认目标有资格进入本阶段；Java 失真时以 Smali 补充，native 位置必须经 library + offset 校验，
不得把反编译 image base 当作相对 offset。

## 工具契约

### structure-aware-retriever

每轮只针对当前 `OPEN` 依赖发起**窄检索**，一次查询只处理一个 requirement：

```text
retrieve_dependency_evidence(
  requirement={id, kind, consumer, need},
  scope={roots, class?, library?, method?},
  limit=8
) -> {location, symbol, snippet, evidence_kind}[]
```

`scope` 必须限定目标类、方法、字段、库或 requirement；`need` 写明精确字段、类型、值、
全局变量或 producer 条件。返回统一为证据数组，单项也用数组，无证据返回空数组。
`location` 为相对源码路径加真实行号，`symbol` 为完整签名或 `jni_name(library + offset)`；
无证据锚点的文本猜测不得进入序列，`text_match` 不能单独证明 producer。

优先复用已定位的类、方法、字段和 native 函数扩大局部上下文，不做无目标全仓扫描。
空结果表示本轮没有证据，不表示依赖不存在；检索环境失败与“检索完成但无 provider”必须分别记录。

### harness-memory

每轮检索新代码前先查询：

```text
component_type=call_sequence
key={package, JNI 签名, library, offset}
dependency={kind, consumer, required_state}
context={参数标签, native-read fields, 已知 binding}
```

只有**已验证且上下文兼容**的序列片段可复用；复用仍须带来源证据、重新执行 local resolution，
不能因 memory hit 跳过门禁。`GlobalReview=ACCEPT` 后才可写入：

```text
component_type=call_sequence, sequence=S_m, annotations=A_m,
dependency_fingerprint, evidence, artifact=call_sequence.md, verification=GLOBAL_REVIEW_ACCEPT
```

失败、部分闭包和未审查片段不得写入。

## 状态与正式流程

维护：

- `S_m = <c1, ..., ck, m>`：前置调用及目标，初始为 `<m>`；
- `A_m`：每个前置调用的 `<call, role, source, binding>` 注解，初始为空；
- `D_m`：当前未满足依赖，初始为 `InitialDependencies(m, Γ, T)`。

```text
S_m = <m>; A_m = {}; D_m = InitialDependencies(m, Γ, T)
while D_m 非空且 budget 未耗尽:
  对每个 OPEN 依赖先查 harness-memory
  对未命中或不兼容的依赖生成窄查询 Q，并取得证据 E
  C = ProposeCalls(E, D_m) 或兼容的 memory 片段
  <S_m, A_m> = InsertAndAnnotate(S_m, A_m, C, E)
  result = CheckLocalResolution(m, S_m, A_m, D_m)
  若 RESOLVED: D_m = {}
  否则: D_m = result.missing_dependencies（包含新调用的参数需求）
若 D_m 非空: 返回 FAIL 或符合门禁的 SKIP
verdict = GlobalReview(m, S_m, A_m)
仅 ACCEPT 时提交 harness-memory 并返回 OK；否则返回 FAIL
```

依赖模型、检索重点和预算见 [references/dependency-analysis.md](references/dependency-analysis.md)。

## 关键操作

### InitialDependencies

同时读取 Java 声明/实参、Smali 调用点和目标 native 实现，至少提取：

- 每个显式参数的来源需求；
- 复杂对象及 native 读取字段等 explicit dependencies；
- opaque handle、native 字段写回、全局状态和 native 返回值等 implicit dependencies。

每条依赖记录 `id / kind / consumer / need / evidence / status / provided_by`。

### ProposeCalls 与 InsertAndAnnotate

候选必须说明满足哪个依赖。producer 插在 consumer 前；新调用自身的参数立即生成新依赖。
每个被接受的调用必须有：

- `role`：如 `IMPLICIT_HANDLE_FACTORY`、`IMPLICIT_GLOBAL_STATE`、`EXPLICIT_FIELD_WRITE`；
- `source`：支持该调用及其作用的 Java/Smali/native 证据；
- `binding`：返回值流向哪个后续参数，或副作用建立哪个字段/全局状态。

缺任一注解、只有名称相似、只有类型匹配或顺序无法解释的候选不得插入。

### Local resolution 与 global review

每轮从审计视角检查 provider 是否真实满足 consumer、绑定方向、新参数闭包、重复/循环及 handle 门禁。
复杂对象无需在本阶段生成代码，但必须形成完整 `CONSTRUCT(class)` 需求，携带 `native_read_fields`
和已发现的 app 实例化证据后交下游；这只是本阶段的显式依赖交接，不代表对象已验证可构造。

本地闭包后再做全局审查：调用顺序、依赖一致性、跨调用 binding、证据完整性、输出可消费性。
任何强依赖仍 `OPEN/BLOCKED`、存在 `UNKNOWN` 或审查不一致，均不得返回 OK。

## 参数终态与硬门禁

允许的参数来源标签为 `FUZZ_INPUT`、`CONSTANT(value)`、`STRING`、`SIMPLE_ARRAY(type)`、
`CONSTRUCT(class)`、`NATIVE_RETURN(func,lib,offset)`、
`OPAQUE_HANDLE(class,factory,lib,offset)`、`FIELD_READ(class,field)`。
`FIELD_READ` 必须继续绑定已证实 writer；`UNRESOLVED_HANDLE` 和 `UNKNOWN` 都是失败诊断，不是成功终态。

**Opaque handle 门禁**：SWIG 三元组、Java 句柄来源链或 native 指针 reinterpret/dereference
任一证据成立，就禁止把该参数标为 `FUZZ_INPUT` 或普通整数。找到工厂则用 `OPAQUE_HANDLE`
并递归解析工厂参数；找不到则为 `UNRESOLVED_HANDLE`，返回 `SKIP`，不得生成下游 harness。

## 输出与返回

按 [references/output-format.md](references/output-format.md) 写 `call_sequence.md`，至少包含：
目标元数据、handle 判定、依赖表、逐轮记录、`S_m/A_m`、参数需求、前置调用表和 global review。
文件中的每个关键结论必须有相对源码锚点；失败时也应保存已完成分析和阻塞原因，不得伪填成功表格。

worker 仅返回一个结构化结果，不写 `summary.json`：

| status | 含义 | repair_owner |
|---|---|---|
| `OK` | `D_m` 已闭合并通过 global review | `null` |
| `FAIL` | 证据不足、预算耗尽或 review 拒绝，可在修复后重试 | 分析修复为 `call-sequence`；检索环境不可用为 `environment` |
| `SKIP` | 目标无资格，或命中 `UNRESOLVED_HANDLE` 等明确硬门禁 | 无资格为 `null`；可补充句柄证据时为 `call-sequence`，并给出 `skip_reason` |

返回字段固定为 `status / stage / artifact / repair_owner / reason`；`stage=call-sequence`。
