---
name: harness-memory
description: 管理 Janus 包级、跨 JNI 的已验证调用序列与参数对象构造 Memory；当 Algorithm 1/2 需要查询可复用组件、提交验证产物、检查兼容上下文或迁移旧 object recipe 时使用。未验证产物不得写入。
---

# Harness Memory Manager

Harness Memory 是单个 APK 包内跨 JNI 共享的组件库，不是聊天摘要，也不是运行时调度状态。
它只保存通过验证的 Algorithm 1 call sequences 与 Algorithm 2 argument recipes，以减少重复检索、
编译和设备验证。

## 存储与唯一写入口

```text
target_APK/<package>/memory/
  index.json
  call_sequences/<name>/<digest>/
  argument_recipes/<name>/<digest>/
```

所有写入使用：

```bash
python3 .agents/skills/harness-memory/scripts/memory_store.py <command> ...
```

不要手工编辑 `index.json` 或直接复制文件到 Memory。查询是只读操作，可以返回多个候选，
由调用算法核对 compatibility context。

## 组件与兼容性 key

### Call sequence

- `kind=call-sequence`，`name` 通常为目标 JNI 完整名。
- compatibility 至少包含 JNI 签名、library、offset、dependency fingerprint、参数标签和
  已知 return/side-effect binding。
- 验证证据必须是当前序列的 `GLOBAL_REVIEW_ACCEPT`；部分闭包和失败分析不能提交。

### Argument recipe

- `kind=argument-recipe`，`name` 为 fully-qualified Java type。
- compatibility 至少包含 constructor/factory 完整签名、native-read fields、子参数依赖、
  Algorithm 1 dependency fingerprint 和 ABI。
- 验证证据必须同时包含 `SYMBOL_CHECK`、`INDEPENDENT_BUILD`、`DEVICE_SMOKE_PASS`。
  同名类型但字段状态或依赖上下文不同不是兼容命中。

直接 primitive/String/array 合成不需要写入 Memory。被复用组件仍需在当前目标中重新检查
上下文与数据流，不能绕过 Algorithm 1 local resolution 或 Algorithm 2 当前计划校验。

## 查询候选

```bash
python3 .agents/skills/harness-memory/scripts/memory_store.py query \
  --package <package> --kind argument-recipe --name <fully.qualified.Type>
```

返回条目包含 digest、compatibility、verification、artifact 路径和内容哈希。先筛选候选，
再用完整 compatibility 精确读取：

```bash
python3 .agents/skills/harness-memory/scripts/memory_store.py get \
  --package <package> --kind argument-recipe --name <type> \
  --compatibility '{"abi":"arm64-v8a","constructor":"<init>(I)V","native_read_fields":[]}'
```

JSON key 会规范化排序；数组顺序有语义，调用方应稳定生成。不要把模糊类型匹配当成 hit。

## 提交已验证组件

先写验证 JSON，例如：

```json
{
  "verification": "SYMBOL_CHECK+INDEPENDENT_BUILD+DEVICE_SMOKE_PASS",
  "evidence": ["target_APK/com.example/pkg/jni/object_recipes/Type/validation.log"]
}
```

然后提交：

```bash
python3 .agents/skills/harness-memory/scripts/memory_store.py put \
  --package <package> --kind argument-recipe --name <type> \
  --compatibility '{"abi":"arm64-v8a","constructor":"<init>(I)V","native_read_fields":[]}' \
  --validation <validation.json> --source-jni <jni> \
  --artifact <construct.c> --artifact <recipe.md>
```

helper 验证元数据、计算内容哈希、复制 artifacts，并在锁内原子更新 index。相同 key 和相同
内容是幂等命中；相同 key 但内容不同会拒绝覆盖，防止无声改变已验证组件。

Call sequence 使用 `kind=call-sequence`、目标 `call_sequence.md` 和
`verification=GLOBAL_REVIEW_ACCEPT`。详细目录和 validation 格式见
[format.md](references/format.md)。

## 旧 recipe 与失效

旧 `<jni>/object_recipes/` 只是迁移候选。重新执行 symbol check、独立构建和设备 smoke，
生成当前验证 JSON 后才可 `put`；不能因为历史 harness 曾 PASS 就直接导入。

Memory 不自动推断失效。APK、library offset、构造签名、字段需求或 ABI 改变会形成不同
compatibility key。发现代码证据与条目不一致时拒绝复用并记录新验证，不覆盖旧证据。
