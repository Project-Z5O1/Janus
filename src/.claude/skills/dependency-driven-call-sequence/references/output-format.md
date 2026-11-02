# call_sequence.md 输出模板

将以下模板写入 `target_APK/<package>/<jni>/call_sequence.md`。无内容的分段保留标题并写“无”；
不得删除失败信息或把占位符留在 OK 结果中。所有路径相对于 `OMEN_ROOT`。

```markdown
# Call Sequence: <jni_name>

- package: <package>
- stage: call-sequence
- target: <java_class>.<java_method><signature>
- jni: <jni_name>
- native: <library> + <offset>
- status: OK | FAIL | SKIP
- verdict: ACCEPT | REJECT | INSUFFICIENT

## Opaque Handle Gate

| parameter | evidence A/B/C | owner class | factory + library + offset | result |
|---|---|---|---|---|
| arg1 | B+C | com.example.Context | Java_... + libx.so + 0x1234 | OPAQUE_HANDLE |

无 handle 时写“无”。存在 `UNRESOLVED_HANDLE` 时，status 必须为 SKIP。

## Dependencies

| id | kind | consumer | need | status | provided_by | evidence |
|---|---|---|---|---|---|---|
| D1 | IMPLICIT_GLOBAL_STATE | target | g_ready != 0 | SATISFIED | step 1 | target_APK/.../libx.c:L120 |

## Resolution Rounds

| round | open dependencies | memory result | narrow queries / evidence | inserted calls | remaining |
|---:|---|---|---|---|---|
| 1 | D1 | miss | global initializer; path:L10-L20 | step 1 | none |

## Annotated Sequence S_m / A_m

| step | call | role | source | binding |
|---:|---|---|---|---|
| 1 | Java_...nativeInit | IMPLICIT_GLOBAL_STATE | path:L10-L20 | writes g_ready required by target |

目标 `m` 是序列末尾，不必伪装成 prerequisite step。

## Argument Requirements

| call | parameter | type | label | source / binding | evidence | construction requirement |
|---|---|---|---|---|---|---|
| target | arg0 | jbyteArray | SIMPLE_ARRAY(byte) | fuzz buffer | path:L30 | 无 |
| target | arg1 | jlong | OPAQUE_HANDLE(...) | step 1 return | path:L42 | 无 |
| target | arg2 | jobject | CONSTRUCT(Config) | downstream construction | path:L55 | native_read_fields=mode; app_instantiation=path:L80 |

`CONSTRUCT` 行必须提供 `native_read_fields`（无则写 none）、已知 app 实例化证据及约束；
这里不生成构造代码。`NATIVE_RETURN` 和 `OPAQUE_HANDLE` 必须指向表内前置步骤。

## Prerequisite Call Table

| step | jni_name | library | offset | 参数来源 | 返回值用途 / 副作用 |
|---:|---|---|---|---|---|
| 1 | Java_...nativeCreate | libx.so | 0x1234 | arg0=STRING(path) | return -> target.arg1 |

前置调用按执行顺序排列，包含完整 JNI 名、library、offset 和每个参数的来源。

## Review and Result

- local_resolution: RESOLVED | UNRESOLVED
- global_review: ACCEPT | REJECT | NOT_RUN
- unresolved: <依赖及原因，或 none>
- memory_commit: COMMITTED | NOT_COMMITTED
- repair_owner: null | call-sequence | environment
- reason: <none、失败原因或 skip_reason>
```

## 填写约束

1. OK 必须同时满足 `local_resolution=RESOLVED`、`global_review=ACCEPT`、`unresolved=none`。
2. 只有 OK 可写 `memory_commit=COMMITTED`；memory 写入失败应改为 FAIL，而不是保留虚假提交记录。
3. FAIL/SKIP 也输出证据和剩余依赖；`global_review` 未执行时写 `NOT_RUN`。
4. `UNKNOWN`、`UNRESOLVED_HANDLE`、`PARAM_PASSTHROUGH` 不得出现在 OK 的参数终态中。
5. worker 返回值与文件中的 `status/stage/repair_owner/reason` 保持一致，但不直接写运行时状态文件。
