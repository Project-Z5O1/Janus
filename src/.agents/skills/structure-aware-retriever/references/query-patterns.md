# 跨语言查询模式

本文件提供查询起点，不要求机械执行全部模式。先用 binding 和完整签名缩小 scope，再选择语义、结构或文本路径。

## 1. 通用执行模板

```text
已知符号       → LSP definition/references → ast-grep 验证代码形态
未知代码形态   → 限定目录的 ast-grep → LSP 核验候选符号
索引/解析失败  → grep 精确标识符 → 回到语义或直接代码证据
Smali native   → binding table → Smali LSP/native export 注释 → native body
```

建议每个窄查询最多返回 8 条；同一 requirement 最多两轮无新证据的扩写。扩写只能改变一个变量，例如同义工厂名或从 Java 回退 Smali，避免无法解释证据来源。

## 2. Native declaration 与 invoke

### Java 结构模式

```text
native $RET $METHOD($$$ARGS)
$RECEIVER.$METHOD($$$ARGS)
$CLASS.$METHOD($$$ARGS)
```

声明与调用分开查询。调用命中后通过 jdtls references 或完整类型检查确认它指向目标 native declaration。

### Smali 文本模式

```text
\.method\b[^\n]*\bnative\b[^\n]*\b<METHOD>\(
invoke-\S+[^\n]*L<OWNER_PATH>;-><METHOD>\(<DESCRIPTOR>\)
```

`<OWNER_PATH>` 使用完整内部类路径，`<DESCRIPTOR>` 包含参数与返回类型。重载必须匹配整条描述符，不能只搜索 `-><METHOD>(`。

## 3. SWIG 与 opaque handle

### Java 结构模式

```text
long swigCPtr
static long getCPtr($CLASS $OBJ) { $$$BODY }
$CLASS.getCPtr($OBJ)
new $CLASS($$$ARGS)
$FACTORY($$$ARGS)
```

结构工具对修饰符或泛型匹配不佳时，可缩小为方法体和字段访问，再用 jdtls 核对声明。

### 精确文本回退

```text
swigCPtr|getCPtr|mNativePtr|mHandle|mContext|mCtx
new_<CLASS>|delete_<CLASS>|<CLASS>_SWIG
```

命名命中只产生候选。强证据需要同时满足以下至少一条链：

- `swigCPtr` 字段 + `getCPtr` 读取 + native factory/delete 对。
- Java handle 字段或 `getCPtr` 的实参链 + 可识别的 native 返回值写入者。
- Native 对参数的显式指针转换/解引用 + factory 返回值到该参数的流向。

### Native 结构/文本模式

```text
reinterpret_cast<$TYPE*>($PARAM)
($TYPE*)(uintptr_t)$PARAM
*($TYPE*)$PARAM
$PARAM->$FIELD
```

Ghidra pseudocode 可能把转换拆成多条表达式；此时返回实际 `native_body` 片段，不为匹配模式而改写代码。发现解引用但未发现 factory 时保持未解决。

## 4. GetFieldID、字段读取与写入

### Native 查询

优先结构查询 JNI 调用，再按 decompiler 形态文本回退：

```text
GetFieldID($$$ARGS)
GetObjectField($$$ARGS)
GetLongField($$$ARGS)
SetObjectField($$$ARGS)
SetLongField($$$ARGS)
GetStaticFieldID($$$ARGS)
SetStaticLongField($$$ARGS)
```

文本标识符集合：

```text
GetFieldID|GetStaticFieldID|Get[A-Za-z]+Field|Set[A-Za-z]+Field
```

对每个字段记录四元组：

```text
<owner class, field name, field descriptor, reader/writer location>
```

`GetFieldID` 的字符串和签名用于关联 Java 字段；真正的 reader 还需 `Get<X>Field`。producer 结论必须有对应 `Set<X>Field`、Java 构造/setter/赋值或已确认 native 写回。只有 `GetFieldID` 不能证明初始化路径。

### Java/Smali 查询

```text
$OBJ.$FIELD = $VALUE
$VALUE = $OBJ.$FIELD
$RECEIVER.set$FIELD($$$ARGS)
```

Smali 回退：

```text
iget[a-z-]*[^\n]*L<OWNER>;-><FIELD>:<TYPE>
iput[a-z-]*[^\n]*L<OWNER>;-><FIELD>:<TYPE>
sget[a-z-]*[^\n]*L<OWNER>;-><FIELD>:<TYPE>
sput[a-z-]*[^\n]*L<OWNER>;-><FIELD>:<TYPE>
```

匹配 owner 和描述符，避免把不同类的同名字段合并。

## 5. RegisterNatives 与 binding

Native 结构/文本入口：

```text
RegisterNatives($$$ARGS)
JNINativeMethod $METHODS[] = { $$$ENTRIES };
```

文本回退：

```text
RegisterNatives|JNINativeMethod|JNI_OnLoad
```

核验步骤：

1. 从方法表读取 `name`、descriptor 和 function pointer 的对应项。
2. 从 `RegisterNatives` 的 class 参数回溯注册目标类。
3. 用完整类、方法、描述符查 `jni_list.json`。
4. 以设备确认的 `library + offset` 定位 native pseudocode。

Parser 已经负责设备上的动态解析；Retriever 不在源码查询阶段重新执行设备注册，也不把源码中的函数指针常量直接当运行时地址。

## 6. Constructor、factory 与 Builder

### Java constructor

```text
new $CLASS($$$ARGS)
$TYPE $VAR = new $CLASS($$$ARGS)
return new $CLASS($$$ARGS);
```

选中候选后，用 jdtls definition 定位实际 `<init>`，再逐个记录实参表达式。构造声明本身不等于 app 内实例化证据。

### Smali constructor

```text
new-instance $REG, L<CLASS>;
invoke-direct {$$$REGS}, L<CLASS>;-><init>(<DESCRIPTOR>)V
```

同一条构造证据应连接 `new-instance`、`<init>` 和后续使用寄存器，不能只凭其中一行。

### Factory/Builder

```text
$TYPE $VAR = $FACTORY($$$ARGS)
return $FACTORY($$$ARGS);
$BUILDER.$METHOD($$$ARGS)
$BUILDER.build($$$ARGS)
```

工厂名未知时，文本回退只在目标类和已知调用者 scope 内使用：

```text
create|newInstance|open|obtain|from|builder|build
```

这些词不提供优先级。接受 factory 必须证明返回类型兼容、结果流向目标，并能递归解决其参数；Builder 还需终止构建调用。

## 7. Algorithm 1 查询清单

按 requirement kind 选择最小证据集合：

| kind | 首轮 scope | 必查模式 | 接受前补证 |
|---|---|---|---|
| `CALL_ARGUMENT` | 目标方法 references | invoke 与实参表达式 | 常量需检查全部已发现调用点 |
| `EXPLICIT_OBJECT` | 参数类型及调用者 | constructor/factory | 类型、字段覆盖、子参数来源 |
| `FIELD_READ` | native 目标函数 | GetFieldID/GetField | 字段 owner、writer 和顺序 |
| `IMPLICIT_HANDLE` | 参数 owner 类与同库 | SWIG/字段链/native cast | factory 及返回值绑定 |
| `IMPLICIT_FIELD_WRITE` | reader 字段的 owner | SetField/iput/setter | 同字段 consumer 与执行顺序 |
| `IMPLICIT_GLOBAL_STATE` | 当前 library | 全局 reader/writer、init 早退 | 同一符号与副作用顺序 |
| `IMPLICIT_RETURN_VALUE` | consumer 实参与兼容 native producer | return、赋值、字段保存 | 完整返回类型和流向 |

候选 producer 必须能回答“满足哪个 requirement、哪条副作用或返回值被谁消费”。无法回答时只保留检索结果，不插入调用序列。

## 8. Algorithm 2 查询清单

1. 先由 Algorithm 2 排除直接类型/JNI 合成和兼容的已验证 memory hit。
2. 用 native reader 生成精确 `required_fields`，不要根据 Java 字段数量猜测需求。
3. 在已知 app 实例化点或目标类 scope 发现 constructor/factory 候选。
4. 对一个候选重新查询完整调用、返回流向和实参。
5. 对每个 `required_fields` 查询构造路径内外的 writer，并核对 owner、descriptor、顺序。
6. 对未解决子参数递归发起新的窄查询；失败后回到候选列表，不降低约束。
7. 最终证据集合覆盖实例化、类型、字段和子参数，再交给 Algorithm 2 组装与验证。

不得仅因某 constructor 接受字节数组就优先选择，也不得把循环依赖默认解析为 NULL。字段没有 writer 证据时保持未覆盖，而不是假设对象内存碰巧可用。

## 9. 结果记录示例

```yaml
- location: target_APK/com.example/src/java/com/example/Decoder.java:64
  symbol: com.example.Decoder.nativeDecode(byte[],long)
  snippet: "return nativeDecode(data, getCPtr(nativeContext));"
  evidence_kind: ast_match
- location: target_APK/com.example/src/native/arm64-v8a/libdecoder.c:418
  symbol: Java_com_example_Decoder_nativeDecode(libdecoder.so + 0x91A0)
  snippet: "ctx = *(NativeContext **)(uintptr_t)handle;"
  evidence_kind: native_body
- location: target_APK/com.example/jni_list.json:17
  symbol: Java_com_example_Decoder_newNativeContext(libdecoder.so + 0x8840)
  snippet: "\"binding_kind\": \"dynamic_register_natives\""
  evidence_kind: binding_table
```

这些结果仍需由 Algorithm 1 建立 producer 返回值到 consumer 参数的绑定；并列展示不等于已经证明调用顺序。
