# Java_qb_blogfuzz_NativeHelper_fuzzMeArray 参数准备计划

## 目标函数
- JNI 名称: `Java_qb_blogfuzz_NativeHelper_fuzzMeArray`
- 库: `libblogfuzz.so` 偏移: `0xC1C`
- Java 签名: `qb.blogfuzz.NativeHelper.fuzzMeArray(byte[])`
- 返回类型: `void`

## 参数解析结果

### arg0: byte[] bArr
- 标签: SIMPLE_ARRAY(byte)
- 来源: 直接从调用者传入的 byte 数组；在唯一调用点中来自 `wrapper.getBuffer()` 的返回值，即 `Arrays.copyOf(mBuffer, mBuffer.length)` 的副本。原始数据为 `"AAAAAAAAAAAAAAAA".getBytes(StandardCharsets.US_ASCII)`（16 字节 ASCII），但内容本质上是任意字节序列，无结构约束。
- 证据:
  - `src/java/sources/qb/blogfuzz/NativeHelper.java:5` — `public native void fuzzMeArray(byte[] bArr);`
  - `src/java/sources/qb/blogfuzz/MainActivity.java:32` — `nativeHelper.fuzzMeArray(wrapper.getBuffer());`
  - `src/java/sources/qb/blogfuzz/Wrapper.java:17-18` — `return Arrays.copyOf(bArr, bArr.length);`
  - `src/smali/smali_classes3/qb/blogfuzz/MainActivity.smali:85-89` — `invoke-virtual {v3}, Lqb/blogfuzz/Wrapper;->getBuffer()[B` → `move-result-object v4` → `invoke-virtual {v1, v4}, Lqb/blogfuzz/NativeHelper;->fuzzMeArray([B)V`
- harness 指令: 从 fuzz buffer 构造 `jbyteArray`：读取前 4 字节作为长度 N（或使用 buffer 全长），调用 `NewByteArray(N)` + `SetByteArrayRegion` 填充内容

## Native 实现行为
- 函数检查 `param_3` 是否为 NULL，若 NULL 则仅打印日志
- 非 NULL 时调用 `GetArrayLength` 获取长度并打印
- 调用 `GetByteArrayElements` 获取元素指针
- 调用 `ReleaseByteArrayElements` 释放（flag=2 即 JNI_ABORT，不回写修改）
- 无内存分配、无指针算术、无内部 helper 调用 — 函数体极简
- 证据: `src/native/arm64-v8a/libblogfuzz.c:66-84`

## 前置调用序列（自动推导）

无前置调用。所有参数均可直接从 fuzz buffer 构造。

| step | jni_name | library | offset | 参数来源 | 返回值用途 |
|-----:|----------|---------|--------|----------|------------|
| 1 | (目标) Java_qb_blogfuzz_NativeHelper_fuzzMeArray | libblogfuzz.so | 0xC1C | arg0: SIMPLE_ARRAY(byte) — 从 fuzz buffer 构造 jbyteArray | — |

## 数据流图

```
fuzz buffer ──→ NewByteArray + SetByteArrayRegion ──→ jbyteArray ──→ fuzzMeArray(jbyteArray)
```

## 闭包检查

| 参数 | 标签 | 已解析 | 备注 |
|------|------|--------|------|
| arg0 (byte[]) | SIMPLE_ARRAY(byte) | ✅ | 直接从 fuzz buffer 构造，无前置依赖 |

- verdict: sufficient
- 未解析项: 无

## 源码锚点
| 证据 | 文件 | 行号 |
|------|------|------|
| native 声明 | `src/java/sources/qb/blogfuzz/NativeHelper.java` | L5 |
| 调用点 (Java) | `src/java/sources/qb/blogfuzz/MainActivity.java` | L32 |
| 调用点 (Smali) | `src/smali/smali_classes3/qb/blogfuzz/MainActivity.smali` | L89 |
| 参数来源 — getBuffer() | `src/java/sources/qb/blogfuzz/Wrapper.java` | L16-18 |
| 原始数据构造 | `src/java/sources/qb/blogfuzz/MainActivity.java` | L26 |
| Native 实现 | `src/native/arm64-v8a/libblogfuzz.c` | L66-84 |
