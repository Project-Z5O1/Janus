---
name: structure-aware-retriever
description: 为 Janus Synthesizer 提供跨 Java、Smali、native pseudocode 的结构感知检索。需要 jdtls/clangd/Smali LSP 语义导航、ast-grep 结构查询、JNI 绑定跳转、依赖证据或递归构造证据，以及 Algorithm 1/2 的窄查询结果时使用。
---

# Structure-aware Retriever

为两个核心算法提供可审计的源码证据，不直接决定最终调用序列或构造计划。

- Algorithm 1：查询未满足依赖的 provider、consumer 与数据流/副作用锚点。
- Algorithm 2：查询直接类型/JNI 合成之外的 constructor、factory、字段写入与子参数来源证据。

所有查询基于 `target_APK/<package>/` 的本地 Parser 产物，不依赖在线代码服务。

## 输入与作用域

查询前确认：

- `src/java/`：递归搜索，兼容实际根位于 `src/java/sources/`。
- `src/smali/`：声明、invoke 和跨语言跳转。
- `src/native/<abi>/`：Ghidra pseudocode。
- `jni_list.json`：首选 binding 数据。
- `signatures_libraries_offsets.txt`：仅在首选数据缺失时回退。

每个查询必须限定目标类、方法、字段、库或 requirement 之一，并给出结果上限。禁止用宽泛关键词整包扫描后拼凑结论。

## 检索顺序

按当前符号已知程度选择最窄路径，并保留实际采用的证据通道：

1. **语义导航**
   - Java 用 jdtls 的 definition、references、document/workspace symbol。
   - Native C/C++ 用 clangd 导航可解析的函数、变量和引用。
   - Smali 用 Smali LSP 解析 declaration、invoke 和 native bridge。
2. **结构匹配**
   - 用 ast-grep 查声明、调用、赋值、构造、类型转换和字段访问形态。
   - 结构命中后核对 owner、参数描述符和类型，不能只匹配方法短名。
3. **文本回退**
   - 语义索引或结构解析不可用时，用 grep 查精确标识符、字段名、JNI API 或描述符。
   - 文本结果用于发现候选；关键结论仍需语义、结构、binding 或直接 native body 证据支撑。

常用模式与 SWIG、字段、动态注册、构造查询示例见 [query-patterns.md](references/query-patterns.md)。

## 统一查询结果

所有接口返回结果数组；单项也使用数组，无证据时返回空数组。每项只使用以下四个字段：

```yaml
- location: target_APK/com.example/src/smali/com/example/Native.smali:87
  symbol: com.example.Native.decode([BJ)I
  snippet: "invoke-virtual {v0, v1, v2}, Lcom/example/Native;->decode([BJ)I"
  evidence_kind: lsp_reference
```

字段要求：

| 字段 | 要求 |
|---|---|
| `location` | 仓库相对路径加真实行号，可追加列号；不得伪造位置 |
| `symbol` | 完整 Java/Smali 签名、native 符号或 `jni_name(library + offset)` |
| `snippet` | 能支持当前结论的简短原文，不改写代码事实 |
| `evidence_kind` | `lsp_definition`、`lsp_reference`、`smali_native_bridge`、`binding_table`、`ast_match`、`native_body`、`text_match` 之一 |

`text_match` 不能单独证明 factory、字段写入、全局状态或 opaque handle 数据流。空数组表示本轮没有证据，不表示依赖不存在。

## Algorithm 1：依赖证据窄接口

对每一个 `OPEN` requirement 单独查询：

```text
retrieve_dependency_evidence(
  requirement = {
    id,                    // 稳定 requirement ID
    kind,                  // CALL_ARGUMENT / EXPLICIT_OBJECT / FIELD_READ /
                           // IMPLICIT_HANDLE / IMPLICIT_FIELD_WRITE /
                           // IMPLICIT_GLOBAL_STATE / IMPLICIT_RETURN_VALUE
    consumer,              // 完整 consumer 符号及参数或状态位置
    need                   // 精确字段、类型、值、全局变量或 producer 条件
  },
  scope = { roots, class?, library?, method? },
  limit = 8
) -> result<location,symbol,snippet,evidence_kind>[]
```

一次调用只处理一个 requirement；发现新依赖后另发查询。扩写同义词或扩大目录前，先保留本轮空结果，避免把搜索扩张误当成新证据。

### 证据质量门槛

| requirement | 可接受证据组合 |
|---|---|
| `CALL_ARGUMENT` | 精确 call site 加对应实参表达式；声明常量时核对全部已发现调用点 |
| `EXPLICIT_OBJECT` | app 内真实 constructor/factory 调用、返回类型兼容性，以及必要字段写入路径 |
| `FIELD_READ` | native/Java reader 锚点；若追踪 producer，还需同 owner+field 的 writer |
| `IMPLICIT_HANDLE` | SWIG/getCPtr/字段链或 native reinterpret 证据，加 factory 返回值到 consumer 的流向 |
| `IMPLICIT_FIELD_WRITE` | 同字段 reader 与 writer 两侧锚点，且写入发生在消费之前 |
| `IMPLICIT_GLOBAL_STATE` | 同一 native 全局状态的 writer 与 reader/早退检查，加调用顺序依据 |
| `IMPLICIT_RETURN_VALUE` | producer 返回类型与返回点，加赋值、字段保存或实参传递到 consumer 的证据 |

接受一个前置调用至少需要 consumer 锚点和 provider 锚点；仅有“某方法被调用”不能证明它满足依赖。关键证据不足时返回现有结果，由 Algorithm 1 保持未闭合，不把文本相似性升级为 producer。

Opaque handle 有解引用证据但未找到 factory 时，查询结果不得被解释为可 fuzz 的整数来源。

## Algorithm 2：构造证据窄接口

直接类型/JNI 合成和已验证 memory hit 由 Algorithm 2 先处理；需要 app 源码证据时才调用本接口：

```text
retrieve_construction_evidence(
  target_type,             // 一个精确目标类型，含必要泛型/数组信息
  candidate,               // 一个 constructor/factory 完整符号；发现阶段可为 null
  required_fields,         // native 实际读取的 owner+field+signature 及 reader 锚点
  constraints,             // 必须保持的值、别名、生命周期和子参数约束
  scope = { roots, class?, method? },
  limit = 8
) -> result<location,symbol,snippet,evidence_kind>[]
```

发现阶段 `candidate=null` 时，只在目标类型及其已知实例化作用域内找候选。选定候选后必须以其完整符号重新窄查；子参数使用新的构造查询，失败则回溯到下一候选。

### 证据质量门槛

- **Constructor**：存在真实 `new` 或 Smali `new-instance` 加 `<init>` 调用，不是只有类声明。
- **Factory/Builder**：存在真实调用，返回类型可赋给目标；Builder 必须有终止构建调用和结果流向。
- **子参数**：实参表达式可继续解析为直接值、JNI 合成、已验证 memory hit 或另一条可递归构造路径。
- **字段覆盖**：`required_fields` 逐项由构造路径、setter、直接赋值或 native 写回覆盖；同名字段不等于同 owner 字段。
- **memory 复用**：兼容性 key 和验证来源由 Algorithm 2 核验；本接口的源码命中不能把旧产物自动升级为已验证组件。
- **失败路径**：证据不满足约束时返回空或剩余候选，由 Algorithm 2 回溯；不得用默认 NULL、任意整数或固定字节数组掩盖失败。

最终选中的构造路径必须同时具备实例化锚点、类型兼容证据和全部必需字段的覆盖证据。容易构造不等于语义正确。

## Smali → native bridge

命中 Smali `.method ... native` 或解析为 native 的 `invoke-*` 时：

1. 用完整 owner、方法名、描述符查询 `jni_list.json`；缺失时回退 `signatures_libraries_offsets.txt`。
2. 取得 `jni_name`、`library` 和相对 `offset`。
3. 在 `src/native/<abi>/<library>.c` 中按 native export 注释和相对偏移定位函数体。
4. 返回 `smali_native_bridge` 结果；函数体证据另返回 `native_body` 结果。

地址规则：

```text
relative_offset = ghidra_entry - image_base
runtime_address = runtime_library_base + relative_offset
```

绝不把 Ghidra image base `0x00100000` 当作运行时 offset。Ghidra entry 只服务于 pseudocode 导航；没有 binding 记录时不得靠地址差猜 JNI 实现。

## 证据审计与停止

- 去重时保留证据更强的结果；同一路径和符号不能只换关键词重复计数。
- 每个关键结论至少有一个强锚点；仅文本命中时明确保留为候选。
- 同一 requirement 连续两轮没有新证据即停止扩张，把空结果交还调用算法。
- 不批量生成调用序列或构造计划，不写 summary，不修改源码和 binding table。
- 保留查询 scope、模式和空结果，供闭包审计与失败回溯复现。

## 客户端可移植性

不同客户端对搜索和 LSP 能力的命名可能不同。按 definition/references、结构匹配、文本搜索等通用能力执行，不调用某一客户端的专有接口。缺少某语言语义服务时沿检索顺序降级，并如实降低 `evidence_kind`，不能声称已完成未发生的语义验证。
