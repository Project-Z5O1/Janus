# argument_plan.md 与 recipe 输出格式

## 1. argument_plan.md

写入 `target_APK/<package>/<jni>/argument_plan.md`。该文件是
`feedback-driven-harness-refiner` 的入口；recipe 路径必须相对于 `OMEN_ROOT`，验证状态不得省略。

````markdown
# Argument Construction Plan: <jni_name>

- package: <package>
- stage: argument-construction
- call_sequence: target_APK/<package>/<jni>/call_sequence.md
- status: OK | FAIL | SKIP
- ABI: <abi>

## Direct Synthesis

| parameter | type | source label | JNI/input action | binding/evidence |
|---|---|---|---|---|
| arg0 | jint | FUZZ_INPUT | read 4 bytes | call_sequence.md argument table |
| arg1 | jstring | STRING | NewStringUTF | call_sequence.md argument table |

无叶参数时写“无”。

## Deterministic Input Layout

| order | consumer.parameter | byte range / length prefix | maximum | constraints | exhaustion behavior |
|---:|---|---|---|---|---|
| 1 | target.arg0 | fixed 4 bytes | 4 | little-endian jint | FAIL_CONSTRUCTION |
| 2 | target.arg1 | 2-byte length + payload | 64 bytes | modified-UTF-8 compatible | FAIL_CONSTRUCTION |

- buffer_size_requirement: <所有固定字段、长度前缀和变长上限之和>
- shared bindings: <数组与 length、offset 与 array.length 等跨参数约束，或 none>

布局必须确定且可边界检查；不得让多个变长字段无界争抢剩余 buffer。输入不足时按表失败并清理，
不得隐式读取越界，也不得用 UNKNOWN/默认 NULL 继续。opaque handle 和 native return 不占 fuzz bytes。

## Construction Decisions

| type | native_read_fields | selected path + signature | app instantiation evidence | field coverage evidence | selection reason |
|---|---|---|---|---|---|
| com.example.Config | mode, sub | Config(int, SubConf) | path:L80-L92 | path:L18-L35 | app 使用且覆盖全部必需字段 |

选择理由必须体现 app 真实路径和字段覆盖，不得写“byte[] 最方便”或“参数最少”。

## Rejected Paths and Backtracking

| type | rejected path | reason | evidence / validation |
|---|---|---|---|
| com.example.Config | Config(byte[]) | 未覆盖 native-read field sub | path:L40-L48 |

## Recursion and Assembly

```text
Config(int, SubConf)
├── int: FUZZ_INPUT
└── SubConf(String, int)
    ├── String: NewStringUTF
    └── int: CONSTANT(0)
assembly: children -> NewObject(SubConf) -> NewObject(Config)
```

记录实际递归树、深度和组装顺序；循环或预算失败必须在 Rejected Paths 中可见，不能用 NULL 截断。

## Verified Recipes

| type | construct.c | recipe.md | symbol check | independent build | device smoke | memory |
|---|---|---|---|---|---|---|
| com.example.SubConf | target_APK/.../construct.c | target_APK/.../recipe.md | PASS | PASS | PASS | COMMITTED |
| com.example.Config | target_APK/.../construct.c | target_APK/.../recipe.md | PASS | PASS | PASS | COMMITTED |

无复杂对象时写“无”，direct-synthesis 结果仍可为 OK。列表顺序必须为 child -> parent。
memory 写入失败时写 `LOCAL_VERIFIED_ONLY` 并说明原因，不得写 COMMITTED。

## Failures / Unresolved Inputs

| item | failure | attempted paths | remaining evidence needed |
|---|---|---|---|
| com.example.Config | NO_VALID_PATH | Config(byte[]), Config() | app path covering field sub |

OK 时写“无”。UNKNOWN、UNRESOLVED_HANDLE 或缺失 S_m binding 必须列在这里，不能转为成功。

## Result

- status: OK | FAIL | SKIP
- repair_owner: null | argument-construction | call-sequence | environment
- reason: <none、失败原因或 skip_reason>
- downstream: feedback-driven-harness-refiner
````

## 2. recipe.md

每个已验证复杂类型在 `object_recipes/<ClassName>/recipe.md` 写：

```markdown
# Construction Recipe: <fully.qualified.Type>

- JNI target: <jni_name>
- selected path: <constructor/factory/instantiation path + descriptor>
- path kind: CONSTRUCTOR | STATIC_FACTORY | APP_INSTANTIATION
- artifact: construct.c

## Evidence

- app instantiation: <relative path:line and summary>
- native-read fields: <fields or none>
- field coverage: <relative path:line and how this path establishes each field>

## Recursive Arguments

| parameter | type | source / child recipe | binding |
|---|---|---|---|

## Input and Ownership

- fuzz consumption: <types and order, or none>
- returned reference: caller releases
- temporary references: released in reverse order
- prerequisites from S_m: <steps or none>

## Validation

- local symbol check: PASS
- independent build: PASS — <artifact/result>
- device smoke: PASS — <device test result>
- ABI: <abi>
- verification context: <native_read_fields + S_m dependency fingerprint>
```

`construct.c` 必须与上述 selected path、descriptor 和 recursion 一致。任何验证项不是 PASS 时，
不得发布该文件组合为 verified recipe，也不得写入 harness-memory。

## 3. 下游消费契约

`feedback-driven-harness-refiner` 只需读取 `argument_plan.md`，按表加载 direct-synthesis 规则和
verified recipes，并保持 child -> parent 组装顺序及 `S_m` binding。它不应重新猜测已否决路径；
若验证证据缺失或文件状态不一致，应退回本技能，而不是默认 NULL 或 UNKNOWN 继续生成。
