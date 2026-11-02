# Target Selection 与 Priority Gate

这两个 gate 用于工程预算分配，不属于 Janus 论文核心算法。它们只能消费已完成的 Parser 产物，不能补充或猜测绑定。

## 一、Target selection

### 资格判定

按 `jni_list.json.functions` 的原始发现顺序逐项检查：

1. **Native declaration**
   - 优先在 `src/java/` 递归定位带 `native` 修饰的方法声明，并核对完整类、方法名和参数类型。
   - Java 缺失或失真时，可用 `src/smali/` 的 `.method ... native` 作为 declaration fallback，记录 `decl_source=smali_fallback`。
   - 找不到声明：`FILTERED_NON_JAVA_NATIVE`。
2. **真实 invoke call site**
   - Java 侧接受经语义引用确认的调用；仅方法短名相同不算证据。
   - Smali 侧接受 `invoke-*`，必须匹配 owner class、方法名和描述符。
   - Java 或 Smali 至少有一条有效调用即可通过；记录实际路径、行号和指令/表达式。
   - 两种源码均无有效调用：`FILTERED_NO_JAVA_CALLSITE`。
3. **零参数规则**
   - 零显式 Java 参数不是过滤条件。无参数方法也可能消费前置调用建立的 handle、字段或全局状态。
   - 参数数量不得改写上述两项资格判定。

每个目标保留 declaration 和 call site 锚点。无法核实重载时先解决签名，不得仅凭同名调用放行。

### targets.md

```markdown
# Targets
- package: <package>
- total_targets: <N>

| order | jni_name | java_class | java_method | params | decl_source | declaration_evidence | callsite_evidence |
|---:|---|---|---|---|---|---|---|
| 1 | Java_com_example_Native_init | com.example.Native | init | void | java | src/java/...:42 | src/smali/...:87 invoke-static |
```

`order` 只保留原始发现顺序，不表示执行优先级。

### filtered.md

```markdown
# Filtered
- package: <package>
- total_filtered: <N>

| order | jni_name | reason | detail |
|---:|---|---|---|
| 4 | Java_com_example_Native_unused | FILTERED_NO_JAVA_CALLSITE | declaration at src/java/...:18; no verified invoke |
```

过滤原因只使用：

- `FILTERED_NON_JAVA_NATIVE`
- `FILTERED_NO_JAVA_CALLSITE`

### 集合同步

- `targets.md` 是后续分析的 allow-list；被过滤项不得进入 Retriever、harness 或 fuzzing。
- `targets.md` 与 `filtered.md` 不得重叠，其并集必须等于本次参与筛选的 binding 集合。
- 通过 helper 将 target order 和过滤终态同步到 summary v2；不要在多个分析分支中分别写状态。

## 二、Priority scoring

只读取 `targets.md`，不得添加被过滤目标，也不得因低分删除目标。主要证据是 `src/native/` 中目标实现及必要的直接 helper；Java、Smali 和 binding metadata 仅用于定位与交叉验证。

### 四个维度

每维取 `0..5`，分数必须由 native 代码证据支持：

| 维度 | 观察内容 |
|---|---|
| `complexity` | 分支、循环、switch、嵌套深度、状态机密度 |
| `pointer_ops` | 指针算术、内存读写/copy、手工缓冲区、类型强转、长度与索引运算 |
| `callee_depth` | 内部 helper 扇出、嵌套调用、间接调用、回调或动态分发 |
| `vuln_surface` | 缓冲区边界、资源获取/释放对称性、整数截断或溢出、类型混淆、格式化串、共享状态、解析与 JNI 资源泄漏 |

评分规则：

```text
priority_score = complexity * 5
               + pointer_ops * 8
               + callee_depth * 3
               + vuln_surface * 9
```

- 满分 125；`HIGH >= 75`，`MEDIUM` 为 `40..74`，`LOW < 40`。
- 反编译实现不可用时四维均为 0，保留在清单中并标为 LOW。
- 极短或纯转发实现不得仅凭函数名、权限或安全用途加分。
- `reason` 用简短事实说明关键分支、内存操作、helper 或漏洞模式，并附 native 锚点。
- 该分数只估计静态 fuzzing 价值，不保证运行时结果。

### priority.md

```markdown
# Priority
- package: <package>
- total_ranked: <N>
- scoring_model: complexity*5 + pointer_ops*8 + callee_depth*3 + vuln_surface*9

| priority_rank | order | jni_name | priority_score | tier | complexity | pointer_ops | callee_depth | vuln_surface | native_evidence | reason |
|---:|---:|---|---:|---|---:|---:|---:|---:|---|---|
| 1 | 3 | Java_com_example_Native_parse | 82 | HIGH | 4 | 4 | 2 | 4 | src/native/arm64-v8a/libfoo.c:310 | 嵌套解析循环；外部长度进入 copy，缺少边界检查 |
```

排序先按 `priority_score` 降序，再按 `targets.md.order` 升序。`priority_rank` 从 1 连续编号。

完成后通过 helper 同步 score、tier、rank、reason 和四维证据。`priority.md` 只提供可审计排序，summary v2 仍是调度状态来源。

## 最终校验

1. 每个保留目标都有 declaration 与至少一个 invoke call site。
2. 零参数目标没有仅因参数数量进入 `filtered.md`。
3. 每个 filtered 条目只有一个规范化原因，并带可诊断 detail。
4. priority 集合与 targets 集合完全一致，分数没有越界，rank 没有重复或断号。
5. 所有优先级理由都能定位到 native pseudocode；没有证据的维度保持低分。
6. gate 写回通过同一 helper 串行完成，manifest 与 summary v2 一致。
