# Java_qb_blogfuzz_NativeHelper_fuzzMeWrapper 参数准备计划

## 目标函数
- JNI 名称: `Java_qb_blogfuzz_NativeHelper_fuzzMeWrapper`
- 库: `libblogfuzz.so` 偏移: `0xD80`
- Java 签名: `qb.blogfuzz.NativeHelper.fuzzMeWrapper(Wrapper)`

## 参数解析结果

### arg0: Wrapper wrapper
- 标签: CONSTRUCT(Wrapper)
- 来源: Java 侧通过 `new Wrapper(byte[])` 构造，构造函数需要一个 `byte[]` 参数
- 证据: `src/java/sources/qb/blogfuzz/Wrapper.java:10` — `public Wrapper(byte[] buffer) throws InterruptedException`
- harness 指令: 需要先构造 Wrapper 对象。Wrapper 的构造函数接受一个 `byte[]`，
  该 byte[] 从 fuzz buffer 提供后通过 `NewByteArray` + `SetByteArrayRegion` 写入 JNI 数组，
  再调用 `Wrapper.<init>(byte[])V` 完成对象构造

#### Wrapper 构造子参数: byte[] buffer
- 标签: SIMPLE_ARRAY(byte)
- 来源: 在调用点中来自硬编码字符串 `"AAAAAAAAAAAAAAAA".getBytes(US_ASCII)`，
  但 fuzz 场景下应从 fuzz buffer 直接构造
- 证据: `src/java/sources/qb/blogfuzz/MainActivity.java:26` — `byte[] buffer = "AAAAAAAAAAAAAAAA".getBytes(StandardCharsets.US_ASCII);`
- harness 指令: 从 fuzz buffer 读取长度和内容，构造 `jbyteArray`，
  作为 Wrapper 构造函数的实参

## 前置调用序列（自动推导）

无前置 native 调用。目标函数仅通过 JNI 调用 Wrapper 对象的 `getBuffer()` 方法读取数据，
不依赖任何其他 native 函数的返回值。

| step | jni_name | library | offset | 参数来源 | 返回值用途 |
|-----:|----------|---------|--------|----------|------------|

（空表 — 无前置 native 调用）

## 数据流图

```
fuzz buffer ──→ byte[] ──→ new Wrapper(byte[]) ──→ fuzzMeWrapper(Wrapper)
                                                   │
                                                   ├─ GetObjectClass(wrapper)
                                                   ├─ GetMethodID("getBuffer", "()[B")
                                                   └─ CallObjectMethod(wrapper, getBuffer) ──→ byte[] → GetArrayLength → ALOGE
```

## Native 实现行为

偏移 `0xD80` 处的反编译 C 代码显示：
1. 检查 wrapper 是否为 null
2. 调用 `GetObjectClass` 获取 Wrapper 类引用
3. 查找 `getBuffer` 方法（签名 `()[B`）
4. 调用 `getBuffer()` 获取 `byte[]` 返回值
5. 读取数组长度并记录日志
6. 清理本地引用

该函数为只读操作，不修改 Wrapper 内部状态，不涉及复杂指针运算或内存管理。

## 闭包检查

| 参数 | 标签 | 已解析 | 备注 |
|------|------|--------|------|
| arg0: Wrapper | CONSTRUCT(Wrapper) | ✅ | Wrapper(byte[]) 构造函数可用 |
| └─ 构造子参数: byte[] buffer | SIMPLE_ARRAY(byte) | ✅ | 从 fuzz buffer 构造 jbyteArray |

- verdict: sufficient
- 未解析项: 无

## 源码锚点

| 证据 | 文件 | 行号 |
|------|------|------|
| native 声明 | `src/java/sources/qb/blogfuzz/NativeHelper.java` | L7 |
| Smali native 声明 | `src/smali/smali_classes3/qb/blogfuzz/NativeHelper.smali` | L33 |
| 调用点 (Java) | `src/java/sources/qb/blogfuzz/MainActivity.java` | L31 |
| 调用点 (Smali) | `src/smali/smali_classes3/qb/blogfuzz/MainActivity.smali` | L82 |
| Wrapper 类定义 | `src/java/sources/qb/blogfuzz/Wrapper.java` | L6 |
| Wrapper 构造函数 | `src/java/sources/qb/blogfuzz/Wrapper.java` | L10 |
| Wrapper.getBuffer() | `src/java/sources/qb/blogfuzz/Wrapper.java` | L16 |
| Native 实现 | `src/native/arm64-v8a/libblogfuzz.c` | L130 |
