# JNI Binding Table 与设备隔离

## 绑定模型

一条绑定连接四类信息：

```text
Java native declaration
  ↔ Smali method/invoke signature
  ↔ device-resolved library + relative offset
  ↔ Ghidra native function body
```

身份键至少包含 `java_class + java_method + params + return_type`。仅按方法短名合并会错误处理重载。

## 静态导出：设备上的 dlsym 解析

适用于导出 `Java_<mangled_class>_<method>` 符号的共享库：

1. 在设备隔离工作区加载目标 `.so`。
2. 用 `dlsym` 解析候选 JNI 符号。
3. 从同一进程的模块映射取得该库基址。
4. 计算 `relative_offset = resolved_address - library_base`。
5. 将符号、库、解析地址、库基址和相对偏移写入提取日志。

这里的 `dlsym` 仅用于 Parser 的设备绑定发现。生成 harness 时仍须通过库基址加相对偏移定位目标，不能复制此解析方式。

## 动态注册：RegisterNatives 解析

适用于 `JNI_OnLoad` 或其他初始化路径调用 `RegisterNatives` 的库：

1. 在设备上执行真实加载与注册路径。
2. 捕获 `JNINativeMethod` 的 `name`、`signature`、`fnPtr` 三元组及注册目标类。
3. 用完整描述符关联 Java/Smali native declaration，不能只凭 `name`。
4. 计算 `relative_offset = fnPtr - containing_library_base`。
5. 记录注册类、描述符、库基址和函数地址作为 `binding_evidence`。

注册未执行、类不匹配或签名不完整时，该动态绑定保持未解决；不得退回按 `Java_*` 命名规则猜地址。

## 每次提取的设备 namespace

复用 `mcptool/jniinfo.py extract` 的 per-run workspace 机制。每次调用，包括同一 APK 的重试，都创建新的运行 token：

```text
/data/local/tmp/offsetFinding/<sanitized_package>_<run_token>/
```

约束：

- 工具编译、APK/库上传、按库解析和结果文件均位于该 run namespace 内。
- 不复用其他 APK 或上一次提取的 app 子目录、二进制和结果。
- 多设备或多次运行不能靠共享根目录同步状态。
- 结束时只删除本次 run namespace；异常退出也执行同样的 owned cleanup。
- 不递归清空共享的 `/data/local/tmp/offsetFinding/` 根目录。
- 拉取结果后核对本地文件属于当前 run，避免旧结果覆盖新失败。

## Ghidra 地址与运行时偏移

Native export 注释应保留：

- `ghidra_entry`：反编译器中的绝对入口地址。
- `relative_offset`：`lib.so + 0x...` 形式的库内偏移。
- `image_base`：本次 Ghidra 导入基址，常见值为 `0x00100000`。

三者的导航关系是：

```text
relative_offset = ghidra_entry - image_base
runtime_address = runtime_library_base + relative_offset
```

`jni_list.json` 的 `offset` 使用经设备绑定确认的相对偏移。Ghidra entry 只用于定位 pseudocode；image base 永远不是运行时 offset，也不能直接交给 harness。

## jni_list.json 记录

推荐结构：

```json
{
  "package": "com.example",
  "total_jni_functions": 1,
  "functions": [
    {
      "jni_name": "Java_com_example_Native_decode",
      "java_class": "com.example.Native",
      "java_method": "decode",
      "params": "byte[],long",
      "return_type": "int",
      "library": "libfoo.so",
      "offset": "0x1234",
      "binding_kind": "static_dlsym",
      "binding_evidence": "device run <run_token>: symbol + module map"
    }
  ]
}
```

动态注册可标记为 `dynamic_register_natives`，静态导出可标记为 `static_dlsym`。若当前提取器只给出统一的设备解析结果，则诚实使用 `device_resolved`，不要根据 JNI 名称猜测注册方式。可以保留原始日志路径或 run token，但不要写入伪造的设备地址。

## 合并与冲突

1. 先按完整 Java 签名分组，再处理同一签名的多条设备记录。
2. 完全相同的库和相对偏移可以去重，并合并证据。
3. 同名不同描述符是不同条目，全部保留。
4. 同签名出现不同库或不同偏移时标记冲突并重新核验，不按静态/动态优先级静默覆盖。
5. `signatures_libraries_offsets.txt` 保留原始文本；结构化修正只写入 `jni_list.json` 并保留理由。
6. Retriever 和 Smali LSP 查绑定优先读 `jni_list.json`，缺失时才回退原始签名文件。
