# 固定模板与代码生成契约

本文是 `feedback-driven-harness-refiner` 的实现细节。若旧产物与本文冲突，以共同输入契约和
固定模板为准，并在 `harness_review.md` 记录迁移差异。

## 1. 生命周期与可编辑区域

每次目标调用的逻辑顺序固定为：

```text
init
  └─ 准备 JVM/JNI 环境、dlopen 依赖、解析函数地址
get_obj
  └─ 解析 buffer、构造参数、执行前置调用、建立对象/句柄状态
call_jni
  └─ 调用目标 JNI、检查异常、记录阶段
release
  └─ 释放计划要求的 native 资源，逆序 DeleteLocalRef
```

`init` 的进程级初始化只执行一次；`get_obj`、`call_jni`、`release` 由固定持久化循环驱动。
不得为绕过失败而提前调用目标、交换前置步骤、省略 release 或增加另一套隐式调用链。

允许修改的内容仅限模板公开的 `GENERATOR` slot：

| Slot | 生成内容 |
|---|---|
| `GENERATOR_FUNCTIONDEFINTION` | 目标、前置和 factory 的函数指针 typedef，签名与元数据一致 |
| `GENERATOR_GLOBALFUNCTIONS` | `prereqFunctionPtrN` / `targetFunctionPtr0` 等直接函数指针变量 |
| `GENERATOR_FUNCTIONOFFSETS` | 库加载、基址查询和相对偏移地址初始化 |
| `GENERATOR_INPUTPARSING` | 输入游标、基本类型/字符串/数组/对象构造及约束检查 |
| `GENERATOR_CALLINGTARGETFUNCTION` | 有序前置调用、覆盖基线处理、数据流和目标调用 |
| `GENERATOR_TARGETLIBRARIES` | 实际链接库和白名单所需库名 |

模板若公开输入尺寸等 GENERATOR 配置槽，也由生成器填写；不得直接改动固定骨架中的等效逻辑。
非 GENERATOR 区域原样继承，包括信号处理、`set_stage`、JNI 异常辅助、覆盖率全局变量和
standalone 入口。固定构建脚本只复制，不重写。

## 2. 函数地址解析

目标函数和每一个前置/factory 函数都必须：

1. 从 `jni_list.json` 与 `call_sequence.md` 核对库名、签名和相对偏移。
2. 用 `dlopen("<lib>.so", RTLD_NOW)` 加载库并检查错误。
3. 用 `get_library_base("<lib>.so")` 从进程 maps 获取运行时基址。
4. 检查基址非 NULL，再把相对偏移加到基址后转换为对应函数指针类型。

```c
void* handle = dlopen("libfoo.so", RTLD_NOW);
if (handle == NULL) {
  /* 记录 dlerror，走模板错误路径 */
}
void* base = get_library_base("libfoo.so");
if (base == NULL) {
  /* 记录基址解析失败，走模板错误路径 */
}
targetFunctionPtr0 = (function_0)((uintptr_t)base + 0x1234);
```

- 示例偏移仅表示“相对偏移”的形式，实际值必须来自当前目标元数据。
- 禁止对目标函数使用 `dlsym`；动态注册或被剥离符号不能依赖导出名称。
- 前置/factory 同样不得凭猜测导出名解析。
- Ghidra 地址若包含 image base，必须先使用元数据中的库相对偏移，不得直接加运行时基址。
- 多库场景分别保存和检查基址，不能复用最后一个库的 base。

## 3. 参数标签与生成方式

| Algorithm 2 标签 | `get_obj` 中的处理 |
|---|---|
| `FUZZ_INPUT` | 按声明类型、字节序、宽度和范围从 buffer 消费 |
| `CONSTANT(value)` | 硬编码计划值，不消耗 buffer |
| `STRING` | 生成长度受限、NUL 终止且满足 `NewStringUTF` 输入要求的字符串 |
| `SIMPLE_ARRAY(type)` | `NewXxxArray` + `SetXxxArrayRegion`，长度由剩余输入和上限共同约束 |
| `CONSTRUCT(class)` | 调用 Algorithm 2 对应构造代码；构造内部临时引用自行清理，返回引用交给 release |
| `NATIVE_RETURN(...)` | 由指定前置调用的返回值提供，不从 buffer 重复构造 |
| `OPAQUE_HANDLE(...)` | 由指定 factory 的前置调用产生；按 factory 签名保持 `jlong`/`jint` 原值传递 |
| `FIELD_READ(class, field)` | 按计划先建立 writer/对象状态，再读取或构造对应值 |
| `UNKNOWN` | 仅采用计划允许的 NULL/零值并标注风险；不得伪造非空指针 |
| `UNRESOLVED_HANDLE` | 阻塞生成，路由 `call-sequence`，禁止编译一个已知无效 harness |

构造代码必须遵守统一游标接口：成功时准确推进消费位置，失败时返回约定错误/NULL，
不越界读取，不遗留临时 local refs。递归构造先满足内部依赖；循环依赖只能采用计划明确允许
的 NULL/默认状态，并在审查报告中暴露覆盖限制。

## 4. 前置调用与 opaque handle

前置步骤严格按 Algorithm 1 的 `step` 顺序执行：

1. 准备该步骤自己的参数；这些参数同样受 Algorithm 2 标签约束。
2. 调用对应 `prereqFunctionPtrN`，随后执行 JNI 异常检查并记录阶段。
3. 将返回值绑定到计划指定的后续参数或资源槽，不按参数位置猜测。
4. factory 失败时执行 Algorithm 2 规定的失败策略。若计划要求有效句柄，则跳过目标并清理；
   若计划明确 fuzz 无效状态，也只能传 factory 实际产生的 NULL/0，不得改用随机整数。
5. 对需要对称释放的 handle/context，在 `release` 调用计划指定的销毁函数，且每次获取至多释放一次。

以下任一情况均为阻塞问题：

- native 解引用、Java `getCPtr`/`swigCPtr`/`mNativePtr` 等证据表明参数是 opaque handle，
  却从 buffer 读取整数；
- 目标需要 handle，但前置表缺少 factory；
- factory 返回值被转换成错误宽度、错误符号或错误对象；
- 为获得更多覆盖而重排初始化和释放顺序。

## 5. 覆盖率去噪

存在至少一个前置 native 调用时，使用模板已有的 `__omen_afl_area_ptr` 和
`__omen_bitmap_snapshot`，位置固定为：

```c
/* 第一个前置调用之前：撤销上一轮目标覆盖，恢复初始基线。 */
if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
  memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
}

/* 有序执行全部前置调用。 */

/* 最后一个前置调用之后、目标调用之前：保存 prereq-only 基线。 */
if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
  memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
}

/* 目标调用。 */
```

NULL guard 必须保留，使无 Frida 的 standalone 运行安全跳过。不得调整两条 memcpy 的顺序，
不得把保存基线移到目标之后。无前置调用时，不生成这两段调用点代码，也不伪造去噪证据。
`afl.js`/`afl-debug.js` 只通过模板规定方式写入真实 area pointer。

## 6. 输入布局与长度约束

Algorithm 2 应给出布局；生成器必须实现并可解释每个字节的归属：

1. 固定宽度字段优先，变长字段顺序固定；多个变长字段使用确定的长度前缀或份额，不允许互相
   无界争抢剩余 buffer。
2. 每次读取前用剩余长度检查，偏移加法不得溢出；固定类型优先用 `memcpy` 等方式避免未对齐
   指针解引用。输入耗尽时使用计划规定的零值、空数组或短字符串，不读取 buffer 外数据。
3. 数组元素数同时受剩余字节、元素宽度、JNI 长度类型和分配上限约束；除法/乘法先检查边界。
   零长度是合法输入，不得因此传入未初始化指针。
4. `NewStringUTF` 不是任意字节容器。按计划过滤/编码输入，处理内嵌 NUL 和非法序列，并记录
   字符约束对覆盖的影响；需要原始字节时改用 byte array，而不是冒险传非法字符串。
5. 对 `offset + length <= array.length`、非负尺寸、枚举范围等约束，在构造时 clamp/拒绝/跳过
   目标，具体行为必须与 Algorithm 2 一致并留下阶段记录。
6. opaque handle、函数指针、native 地址和资源销毁时机永远不是普通可变字段。

`BUFFER_SIZE` 是所有前置和目标参数的最大布局需求之和，包含长度前缀与对齐/保留字节，并受
模板允许的上限约束。它必须同步到：

- `fuzz.c` 的实际读取上限；
- `afl.js` 与 `afl-debug.js` 的 buffer 长度；
- `fuzzing.sh`、`fuzzing-debug.sh` 及验证命令的 `-G`。

三者不一致、尺寸缺失或解析器可能读取超过 `-G` 时，按 `TEMPLATE_BUILD` 或
`INPUT_CONSTRAINT` 回流，禁止静默使用 256 等默认值。

## 7. JNI 引用与 release

- 所有 `New...`、`FindClass`、`GetObjectClass`、构造代码返回的 local ref 都登记创建顺序。
- 在目标调用完成或按计划提前跳过后，统一进入 `release`；正常、JNI 异常和参数构造失败路径
  都释放已创建资源。
- `DeleteLocalRef` 严格逆创建顺序，并在非 NULL 时调用；构造代码内部临时引用由构造代码释放。
- native factory/destructor、文件/缓冲区释放与 local ref 清理按 Algorithm 1 的资源依赖排序，
  不把“逆序 local refs”误用为逆序调用任意 native destructor。
- release 必须先验使用 release profile 验收；只有失败后才用 debug profile 收集证据。修复后
  再次通过 release，不得以 debug 的结果替代。

## 8. 语义审查最低证据

`harness_review.md` 至少记录：

| 维度 | 必查证据 |
|---|---|
| 调用路径 | 前置步骤数量、顺序、函数偏移、参数绑定和返回值用途逐项对照 |
| 参数解析 | 每个 JNI 参数的标签、buffer 区间、类型宽度、常量及长度约束 |
| 前置条件 | 对象初始化、句柄工厂、writer、factory 失败策略和资源对称性 |
| 异常/清理 | 每个 JNI API 与前置/目标调用后的异常检查，所有退出路径的释放顺序 |
| 覆盖潜力 | 目标库白名单、可变异分支、被常量或状态限制的路径、coverage reset |
| 模板一致性 | 固定区域未改、函数地址解析正确、尺寸同步、release/debug 使用顺序 |

结论规则：

- `APPROVED`：无阻塞 FAIL；剩余 WARN 不影响调用有效性，并已明确记录。
- `NEEDS_REVISION`：存在可定位、可修复且需重新验收的问题。
- `REJECTED`：签名/调用路径根本错误、opaque handle 被 fuzz、目标库未插桩或其他会使
  fuzzing 无效/产生误报的问题。

审查不能只看生成代码自洽；至少要与 Algorithm 1/2 对照，必要时回到 Java、Smali 和 native
实现验证。编译成功或 standalone 退出 0 均不能代替语义审查。
