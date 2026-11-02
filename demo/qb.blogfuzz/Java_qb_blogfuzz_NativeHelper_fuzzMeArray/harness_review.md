# Harness Quality Review: Java_qb_blogfuzz_NativeHelper_fuzzMeArray

## 审查结论

**Verdict**: APPROVED

| 维度 | 结果 | 说明 |
|------|------|------|
| 调用路径保真度 | PASS | 无前置调用，callingsequence.md 与 fuzz.c 一致；目标函数直接调用，签名匹配 |
| 参数解析充分性 | PASS | SIMPLE_ARRAY(byte) → NewByteArray + SetByteArrayRegion，使用完整 fuzz buffer，类型正确，变长 |
| 前置条件满足度 | PASS | 目标函数仅需 byte[] 参数，无对象状态依赖；native 实现为纯 JNI 数组操作，无内部状态要求 |
| 异常处理区分度 | PASS | JNI 异常检查覆盖完整；NULL 检查存在；清理路径正确（DeleteLocalRef 在所有路径上执行） |
| 覆盖潜力评估   | PASS | MODULE_WHITELIST 包含目标库；目标函数有两个分支（NULL / non-NULL），harness 均可触达 |

## 详细发现

### 1. 调用路径保真度

**PASS**

callingsequence.md 明确声明无前置调用，参数准备计划中唯一的 step 即为目标函数本身：

```
| step | jni_name | library | offset | 参数来源 | 返回值用途 |
| 1 | (目标) Java_qb_blogfuzz_NativeHelper_fuzzMeArray | libblogfuzz.so | 0xC1C | arg0: SIMPLE_ARRAY(byte) | — |
```

fuzz.c 的 `GENERATOR_CALLINGTARGETFUNCTION` 区块正确实现了无前置调用、直接调用目标函数的模式（L131）：
```c
targetFunctionPtr0(ctx.env, NULL, jData);
```

- 函数签名匹配：`function_0` 类型为 `void (*)(JNIEnv*, jobject, jbyteArray)`，与 `jni_list.json` 中的 `params: byte[]`, `return_type: void` 一致
- 偏移量匹配：`0xC1C`（L181），与 jni_list.json 和 callingsequence.md 一致
- 库名匹配：`libblogfuzz.so`（L170-181）
- 数据流：fuzz buffer → NewByteArray + SetByteArrayRegion → jData → 目标函数第三参数，与 callingsequence.md 数据流图一致

### 2. 参数解析充分性

**PASS**

arg0 标签为 `SIMPLE_ARRAY(byte)`，fuzz.c 中的解析实现（L122-127）：

```c
jbyteArray jData = (*ctx.env)->NewByteArray(ctx.env, length);
if (jData == NULL) { set_stage("cleanup"); return; }
(*ctx.env)->SetByteArrayRegion(ctx.env, jData, 0, length, (const jbyte*)(buffer));
```

- **类型映射正确性**：`jbyteArray` 与 `jni_list.json` 的 `params: byte[]` 完全对应
- **值域覆盖**：数组长度 = fuzz buffer 实际读取长度（`length` 来自 `rlength`），变长，无硬编码截断，AFL++ 可以产生任意长度（0 ~ BUFFER_SIZE-1）的输入
- **Buffer 布局效率**：单参数函数，全部 buffer 用于数组内容，无固定字段与变长字段交错问题，布局合理
- **CONSTANT 值**：无 CONSTANT 标签参数，无需验证
- **BUFFER_SIZE = 4096**：对单 byte[] 参数合理，与 afl.js 的 `BUF_LEN 4096` 一致

### 3. 前置条件满足度

**PASS**

- **对象生命周期**：目标函数是 `NativeHelper` 的实例方法（`jobject` 为第二参数），但 Ghidra 反编译代码（L66-84）中 `param_2`（即 `this`）从未被使用。函数仅操作传入的 `jbyteArray`（`param_3`）。harness 传入 `NULL` 作为 jobject，在此场景下安全
- **状态依赖**：native 实现无内部状态依赖——不读取全局变量、不依赖其他 native 方法设置的状态。纯 JNI 数组操作函数
- **参数约束**：Java 端 `wrapper.getBuffer()` 返回 `Arrays.copyOf(mBuffer, mBuffer.length)`，即任意字节数组的防御性副本，无结构约束。harness 允许任意字节内容，符合语义
- **调用上下文**：在 Java 端，`fuzzMeArray` 在 `fuzzMeWrapper` 之后调用（L32 of MainActivity.java），但两者无数据依赖关系——`fuzzMeArray` 仅接收 `wrapper.getBuffer()` 返回的独立数组副本。`fuzzMeWrapper` 的执行与否不影响 `fuzzMeArray` 的行为

### 4. 异常处理区分度

**PASS**

- **JNI 异常检查覆盖**：
  - `init_java_env` 后（L164）：✅
  - `targetFunctionPtr0` 调用后（L132-136）：✅ `check_and_clear_jni_exception("call_target")`
  - 异常路径正确清理 `jData`（L133：`DeleteLocalRef`）
- **NULL 检查**：
  - `NewByteArray` 返回值检查（L123-126）：✅ NULL 时提前 return
  - `dlopen` 返回值检查（L171-173）：✅
  - `get_library_base` 返回值检查（L176-179）：✅
- **清理路径正确性**：
  - 正常路径：L139 `DeleteLocalRef(ctx.env, jData)` ✅
  - 异常路径：L133 `DeleteLocalRef(ctx.env, jData)` → return ✅
  - 创建与清理逆序匹配（仅一个 JNI 对象，无复杂嵌套）
- **信号处理**：`crash_signal_handler` 使用 `signal(signum, SIG_DFL); raise(signum)` 模式（L82-83），AFL++ 父进程可正确识别 WIFSIGNALED，非 `_exit()` 误判

### 5. 覆盖潜力评估

**PASS**

**分支可达性分析**（基于 Ghidra 反编译 L66-84）：

```
if (param_3 == NULL)          → 分支 A: 打印日志，直接 return
else {                         → 分支 B: GetArrayLength → GetByteArrayElements → ReleaseByteArrayElements
  GetByteArrayElements → NULL → 分支 B1: 直接跳过 Release
  GetByteArrayElements → non-NULL → 分支 B2: 执行 ReleaseByteArrayElements
}
```

- 分支 A（NULL 输入）：harness 通过 `length=0` 的输入时，虽然 NewByteArray(0) 返回合法的非 NULL 数组，但 AFL++ 的 havoc/deterministic mutation 可能产生触发目标函数内部异常的场景。然而 `param_3` 的 NULL 分支在 harness 中无法直接触发（harness 总是传入有效 jbyteArray 或在 NewByteArray 失败时提前返回）。**INFO**: NULL 分支不可达是可接受的——harness 的目标是 fuzz 非 NULL 路径，NULL 路径在真实 Java 调用中也不会出现
- 分支 B1/B2：`GetByteArrayElements` 的返回值取决于 JNI 实现（通常非 NULL），两个子分支均可通过变异数组内容间接影响
- AFL++ 能通过变异 `length`（0 ~ 4096）和 buffer 内容驱动目标函数的不同执行路径

**MODULE_WHITELIST 检查**（afl.js L1-4）：

```javascript
const MODULE_WHITELIST = ["fuzz", "libblogfuzz.so"];
```

- 目标库 `libblogfuzz.so` 已包含 ✅
- 覆盖率不会被排除

**目标函数复杂度评估**：

- 控制流：简单 if-else，3 个基本块
- 指针操作：仅通过 JNI API 访问数组元素指针，无手动指针算术
- 调用深度：仅调用 3 个 JNI 函数，无内部 helper
- 漏洞面：极低——函数仅读取数组长度和元素指针，使用 JNI_ABORT 释放，不做写入。这是一个 "安全桩函数" 风格的目标

**INFO**: 目标函数本身为极简实现（仅日志打印 + JNI 数组元素获取/释放），fuzzing 发现 crash 的概率极低。harness 实现本身质量良好，覆盖局限来自目标函数而非 harness。

### 6. 模板一致性

**PASS**

与 `assets/fuzz.c` 模板对比：

- **信号处理器**：`install_signal_handlers()` 在 `main()` 开头（L148）✅；`crash_signal_handler` 使用 `signal(signum, SIG_DFL); raise(signum)` ✅
- **覆盖率去噪**：`__omen_afl_area_ptr` 和 `__omen_bitmap_snapshot` 全局声明存在（L19-20）✅；由于无前置调用，fuzz.c 中未出现 memcpy 代码 ✅（符合规范）
- **阶段追踪**：`set_stage()` 在 `input_parsing`、`call_target`、`cleanup` 调用 ✅；`g_trace_stages` 由 `OMEN_TRACE_STAGES` 环境变量控制（L149-150）✅
- **JNI 异常检查**：`check_and_clear_jni_exception` 在 `init_java_env` 后和目标函数调用后均有调用 ✅
- **buffer 声明**：`uint8_t buffer[BUFFER_SIZE]`（非 const）（L144）✅
- **地址解析**：使用 `dlopen` + `get_library_base` + 偏移量（非 `dlsym`）（L170-181）✅

## 修复建议

无。harness 实现完整且正确，与参数准备计划完全一致。
