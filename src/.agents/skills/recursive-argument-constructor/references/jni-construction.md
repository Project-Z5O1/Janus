# JNI 构造与独立验证

## 1. 叶类型合成

| 类型/标签 | JNI 或输入动作 | 约束 |
|---|---|---|
| primitive + `FUZZ_INPUT` | 从 buffer 按类型宽度解码并推进 offset | 不越界；遵守长度、枚举等已识别关系 |
| primitive + `CONSTANT(v)` | 写入常量 `v` | 不再消耗 fuzz buffer |
| `String` / `STRING` | `NewStringUTF` | 检查 NULL 与 exception |
| primitive array / `SIMPLE_ARRAY(t)` | `New<t>Array` + `Set<t>ArrayRegion` | 长度来自已解析来源，元素不越界 |
| `NATIVE_RETURN` / `OPAQUE_HANDLE` | 使用 `S_m` 中已绑定的前置返回值 | 不递归伪造对象，不改 fuzz 整数 |

reference 类型即使是 array，也必须检查分配结果。buffer 不足、offset 溢出或 Java exception 都是构造失败，
不能通过缩短到零长度或默认 NULL 假装成功，除非 `call_sequence.md` 有明确证据要求该值。

## 2. Recipe 接口

每个复杂类型提供独立函数，推荐接口：

```c
jobject construct_<ClassName>(JNIEnv* env,
                              const uint8_t* buffer,
                              size_t length,
                              size_t* offset);
```

约定：

- 返回新建本地引用，调用方负责 `DeleteLocalRef`；
- 失败返回 NULL，并保留明确错误日志；不得把失败 NULL 写入成功 recipe；
- 函数内临时 class、method result、array 和不再需要的 child refs 逆序释放；
- child object 至少存活到 parent 的 `NewObject`/factory 调用结束；
- 所有 offset 更新先检查 `length - *offset`，防止回绕；
- 多 child 共享或重复引用必须遵循 `S_m` binding，不擅自复制对象。

若简单类名在一个 JNI 下冲突，输出目录和函数名使用可逆的限定名消歧，并在 `argument_plan.md` 记录映射。

## 3. 自底向上组装

### Constructor path

1. 用完整内部类名 `FindClass`；
2. 用精确 descriptor `GetMethodID(cls, "<init>", sig)`；
3. 按 `ParamTypes(pi)` 顺序准备所有 child；
4. 调用 `NewObject` 并立即检查 exception 与返回值。

### Static factory path

1. 确认 app 中真实 factory 的 owner、name、descriptor 与返回类型；
2. `GetStaticMethodID`；
3. 自底向上准备参数后调用 `CallStaticObjectMethod`；
4. 检查返回对象确实可赋给 `tau`，并检查 exception。

### App instantiation path

若证据指向 instance factory 或 Builder，先构造 receiver/Builder，再按真实调用顺序执行 object methods。
不得仅凭“常见模式”添加 build/create 方法；最终返回类型和每一步 signature 都要核对。

本流程不增加独立 residual-field completion 阶段。字段覆盖是选择 `pi` 的依据，而不是在任意对象后
随机补写字段的许可。所选路径不能建立必需状态时，应 backtrack。

## 4. Local symbol check

在生成测试程序前逐项核对：

- 类存在于 Java 或 Smali，内部名和包名一致；
- `<init>`、factory、Builder 方法的 owner/name/descriptor 完全匹配；
- static 与 instance JNI lookup API 没有混用；
- 每个 child 的 JNI 类型与 descriptor 相符；
- `native_read_fields` 的覆盖结论有所选路径的代码证据；
- 所有 `NATIVE_RETURN/OPAQUE_HANDLE` 都能绑定到 `S_m` 的已验证前置步骤。

符号不存在或证据矛盾时，当前候选失败并 backtrack；不得靠运行时清除 `NoSuchMethodError` 继续。

## 5. 独立编译与设备 smoke test

对每个 recipe 建立独立测试，不依赖完整 harness：

1. `test_construct.c` 初始化 JNI 环境，调用当前 `construct_<ClassName>`，检查非 NULL、
   `ExceptionCheck` 为 false、必要时用 `IsInstanceOf` 检查类型，最后释放引用；
2. 单独交叉编译该测试与 recipe；constructor-only 测试可只链接 JNI 环境库，factory 或类初始化
   有额外依赖时按证据加入最小库集合；
3. 在设备加载真实 APK 类与必要依赖，执行 smoke test。若 `S_m` 明确声明类初始化前置调用，
   测试按序包含这些调用，但不调用目标 JNI；
4. 只有编译成功、进程正常退出、对象创建成功且全程无未处理 Java exception，才算通过。

设备目录必须由 package/JNI/class 唯一定位。使用仓库现有 helper：

```bash
DEVICE_DIR="$(python3 "$OMEN_ROOT/.agents/skills/recursive-argument-constructor/scripts/recipe_device_dir.py" \
  --package <package> --jni-name <jni> --class-name <ClassName>)"
```

验证后清理该次设备目录。设备未连接、未执行、崩溃、超时或只完成编译都不能记为 smoke pass。

child recipe 先验证，parent 后验证；parent smoke 失败时可在剩余预算内 backtrack 到其他 parent path，
不得修改已验证 child 的语义来掩盖 parent 状态问题。

## 6. 发布与 memory commit

验证成功后才发布最终 `construct.c` 和 `recipe.md`，并向 `harness-memory` 提交：
类型、完整路径签名、子参数依赖、字段覆盖、证据锚点、ABI、测试命令/结果和 artifact 路径。

编译或 smoke 失败的临时代码只能作为失败诊断，不得被标记 verified、不得作为 memory hit 返回。
memory 写入失败时，`argument_plan.md` 必须如实记录；recipe 可保留为本地已验证产物，但不得声称已提交。
