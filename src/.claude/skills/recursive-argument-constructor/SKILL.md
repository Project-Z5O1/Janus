---
name: recursive-argument-constructor
description: 递归生成并验证 JNI 复杂参数对象的 constructor/factory 构造路径与 recipe；当 call_sequence.md 含 CONSTRUCT 需求、需要递归解决子参数、独立 smoke test 或生成 argument_plan.md 时使用。
---

# Algorithm 2: Recursive Argument Object Construction

## 目标与边界

输入目标参数类型 `tau`、Algorithm 1 的 `S_m`、反编译代码 `Gamma` 和工具集 `T`，
为每个复杂参数生成可执行、已验证的 JNI 构造代码；无法构造时明确失败。

- 状态阶段名：`argument-construction`
- 输入：`target_APK/<package>/<jni>/call_sequence.md`
- 主输出：`target_APK/<package>/<jni>/argument_plan.md`
- 已验证 recipe：`target_APK/<package>/<jni>/object_recipes/<ClassName>/construct.c`
  与 `recipe.md`

输出供 `feedback-driven-harness-refiner` 组合进 harness。本技能不生成完整 `fuzz.c/afl.js`，
不改变调用序列，不运行 fuzzing，也不直接修改 `summary.json`。

## 输入门禁

只消费 `call_sequence.md` 中 `status=OK` 且 `global_review=ACCEPT` 的序列。逐项读取参数标签、
前置调用 binding、复杂对象的 `native_read_fields` 与 app 实例化证据。

- `UNRESOLVED_HANDLE`：停止，禁止 fuzz 句柄或猜测构造；
- `UNKNOWN`、缺失来源、未闭合 `NATIVE_RETURN`：停止并退回 `dependency-driven-call-sequence`；
- opaque handle 使用 `S_m` 已绑定的 factory return，不在本技能伪造 Java 对象；
- 无复杂对象时仍输出 direct-synthesis `argument_plan.md`，不创建空 recipe。

## 工具契约

### structure-aware-retriever

对每个非叶类型检索 constructors、static factories 和 app 内真实 instantiation sites，并查询这些路径
写入的字段及子参数来源。发现阶段与候选复核均使用统一窄接口：

```text
retrieve_construction_evidence(
  target_type,
  candidate,               // 发现时为 null；复核时为完整 constructor/factory 符号
  required_fields,         // native 实际读取的 owner+field+signature 及 reader 锚点
  constraints,             // 值、别名、生命周期、S_m binding 与子参数约束
  scope={roots, class?, method?},
  limit=8
) -> {location, symbol, snippet, evidence_kind}[]
```

候选必须经真实实例化锚点、返回类型兼容和字段覆盖证据复核。发现候选后，以其完整符号再次窄查；
不能只按方法名或参数便利性选路。空数组不证明构造路径不存在，证据不足时继续有界窄检索或判失败，
不补写猜测。

### harness-memory

直接叶类型合成之后、构造路径检索之前查询：

```text
component_type=argument_recipe
key={package, fully_qualified_type}
context={constructor/factory 签名, native_read_fields, S_m dependency fingerprint, ABI}
```

仅复用 `verification=SYMBOL_CHECK+INDEPENDENT_BUILD+DEVICE_SMOKE_PASS` 且 context 兼容的 recipe；
类型名相同但字段状态、构造签名或依赖上下文不兼容时视为 miss。未验证或验证元数据缺失的条目不得使用。

只有当前 recipe 的 local symbol check、独立编译和设备 smoke test 全部成功后，才可写入：

```text
component_type=argument_recipe, type, path_signature, arg_dependencies,
native_read_fields, artifacts=[construct.c, recipe.md], evidence, verification, ABI
```

## 正式算法

严格按以下顺序执行；`LocalCheck` 包含符号检查、独立编译和设备 smoke test：

```text
RecConstruct(tau, S_m, Gamma, T):
  若 Synthesizable(tau): 返回 EmitJNISynthesis(tau)
  若 Memory.Hit(tau, compatible_context): 返回 Memory.Get(tau)
  若 tau 已在当前 recursion stack 或 depth/node budget 耗尽: 当前路径 FAIL

  ctorCands = QueryConstructorsAndFactories(tau, Gamma, T)
  rankedPaths = SelectConstructionPaths(ctorCands, S_m, Gamma)

  对每个候选路径 pi（按 rankedPaths 顺序）:
    argCodes = []
    对 ParamTypes(pi) 中每个 tau_prime:
      ctor_child = RecConstruct(tau_prime, S_m, Gamma, T)
      若 ctor_child == FAIL: 放弃 pi，backtrack 到下一候选
      argCodes.add(ctor_child)

    ctor_tau = EmitConstruction(pi, argCodes)
    若 LocalCheck(ctor_tau) 失败: 放弃 pi，backtrack 到下一候选
    Memory.Put(tau, ctor_tau, verified_context)
    返回 ctor_tau

  返回 FAIL
```

实现细节见 [references/jni-construction.md](references/jni-construction.md)。

## 路径选择与递归规则

### 1. 直接合成终止

primitive 从 fuzz buffer 或 `CONSTANT` 解码；`String` 用 `NewStringUTF`；primitive array 用
`New<Primitive>Array` 与 `Set<Primitive>ArrayRegion`。遵守 `S_m` 的常量、长度和返回值 binding，
不得把 `UNKNOWN` 当叶类型，也不得将 handle 当 primitive 合成。每个 fuzz 消耗字段都须在计划中
给出确定顺序、长度前缀/上限、跨参数约束、输入耗尽行为和总 buffer 尺寸需求。

### 2. 状态感知选路

候选排序同时依据：

1. app 是否真实使用该 constructor/factory/instantiation path，目标参数是否来自该路径；
2. 路径对 `native_read_fields` 的覆盖是否有代码证据；
3. 子参数依赖能否由 `S_m`、直接合成或递归构造闭合。

**不得固定优先 `byte[]` constructor、无参 constructor 或参数最少路径。** 这些仅是候选；
若简单路径没有建立 native 所需状态，应让位于 app 真实且字段覆盖更完整的路径。

不执行 residual-field completion 作为补洞步骤。必需字段未被候选路径覆盖时，换候选或 FAIL；
不得假定无参构造后再随机写字段等价于 app 状态。

### 3. 子参数递归与 backtrack

子参数沿相同算法递归，先得到所有 child，再组装 parent。任何 child 失败、深度/循环预算触发、
符号检查或验证失败，都放弃当前路径并尝试下一候选；不得用默认 `NULL` 填 child 来终止循环。

默认预算：最大递归深度 6、每类型最多 8 条候选路径、每次目标参数构造最多 32 个 object nodes。
当前 recursion stack 再次出现同一类型即视为循环；只有存在不涉及该循环的真实替代路径才可继续。

### 4. 自底向上 JNI 组装

- constructor：`FindClass` + `GetMethodID("<init>", sig)` + `NewObject`；
- static factory：`FindClass` + `GetStaticMethodID` + `CallStaticObjectMethod`；
- app 实例化路径包含 instance factory/Builder 时，先递归构造 receiver，再用对应 object method 完成。

每次调用检查返回引用与 Java exception；临时 local refs 按逆序释放，返回对象由调用方释放。
所有 child 必须先于 parent 构造并保持到 parent 调用完成。

## 验证、发布与返回

每个复杂类型先独立验证，再发布 `construct.c/recipe.md`；依赖类型按 child → parent 顺序验证。
symbol check 通过但编译或 smoke 失败仍是 FAIL，不得写 harness-memory，也不得标为 verified。
若设备不可用或 smoke 未实际执行，状态只能是未验证，不能返回含已验证 recipe 的 OK。

按 [references/output-format.md](references/output-format.md) 写 `argument_plan.md`，记录路径选择、
递归树、字段覆盖、验证证据、recipe 列表和失败原因，供 `feedback-driven-harness-refiner` 消费。

worker 返回 `status / stage / artifact / repair_owner / reason`，且 `stage=argument-construction`：

| status | 含义 | repair_owner |
|---|---|---|
| `OK` | 所有必需参数均已直接合成、复用已验证 recipe，或生成并验证新 recipe | `null` |
| `FAIL` | 所有路径失败、预算耗尽、存在 UNKNOWN，或编译/smoke 未通过 | 构造或验证问题为 `argument-construction`；上游需求缺陷为 `call-sequence`；工具/设备环境不可用为 `environment` |
| `SKIP` | 目标无资格进入本阶段 | `null`，附 `skip_reason` |

不直接写 `summary.json`；部分成功、默认 NULL 补洞和 UNKNOWN 均不得包装成 OK。
