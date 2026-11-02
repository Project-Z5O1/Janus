# 依赖模型、检索与预算

## 1. 两类依赖

### Explicit dependencies

可由 Java 侧建立，不跨越 JNI 边界：

- 参数对象需要 constructor、factory 或 app 内 instantiation path；
- native 将读取的字段必须由所选 Java 路径初始化；
- 调用点具有一致常量、枚举、数组长度或其他参数约束。

本阶段恢复这些约束并形成参数需求。复杂对象的代码生成与运行验证属于
`recursive-argument-constructor`，不能在这里假装已经完成。

### Implicit dependencies

必须由前置 JNI/native 执行产生：

- 实为 C/C++ 指针的 `jlong/jint` opaque handle；
- native 通过 `Set<Field>` 写回、目标随后读取的字段；
- native 全局初始化标志、上下文表、缓存或单例；
- 前置 native 返回值流向后续参数；
- 目标早退条件所要求的库内初始化或资源创建副作用。

Java constructor/setter 不能冒充 implicit native provider；名称相似或返回类型相同也不能证明满足依赖。

## 2. Requirement 记录

| 字段 | 内容 |
|---|---|
| `id` | 稳定编号，如 `D1` |
| `kind` | `EXPLICIT_OBJECT`、`FIELD_READ`、`IMPLICIT_HANDLE`、`IMPLICIT_FIELD_WRITE`、`IMPLICIT_GLOBAL_STATE`、`IMPLICIT_RETURN_VALUE`、`CALL_ARGUMENT` |
| `consumer` | 目标或前置调用的参数、字段、全局状态 |
| `need` | 必须建立的具体状态 |
| `evidence` | 相对路径、行号与片段 |
| `status` | `OPEN`、`SATISFIED`、`DELEGATED`、`BLOCKED` |
| `provided_by` | 调用、参数标签或完整构造需求 |

`D_m` 只包含仍未闭合的项。`DELEGATED` 仅用于信息完整的复杂对象构造需求交接；opaque handle、
native 字段写回、全局状态和 native 返回值不得以此状态绕过闭包。

## 3. InitialDependencies 证据

1. **Java/Smali 声明与调用点**：确定参数列表、实参表达式、常量、字段读取、方法返回值和上层形参。
2. **Native 实现**：检查 `GetFieldID/Get*Field/GetObjectField`、`Set*Field`、字符串/数组访问、
   指针 cast/dereference、全局状态检查及早退条件；必要时读取直接 helper。
3. **Cross-language 绑定**：核对 Java 声明、Smali invoke、JNI 名、library 和 offset。
   不能锚定的猜测只可成为查询线索，不可成为强依赖或 accepted call。

## 4. Opaque handle 门禁

对 `jlong/jint` 参数，以下任一组证据成立即认定为 opaque handle：

- **A：SWIG 布局**：`swigCPtr`、`static long getCPtr(Class)`、`new_<Class>` 与
  `delete_<Class>` 工厂/析构组合形成一致证据链；
- **B：Java 来源链**：实参来自 `Class.getCPtr(...)`、`swigCPtr`、`mNativePtr`、
  `m*Ptr/m*Handle/m*Ctx`，或字段由 native factory 返回值写入；
- **C：Native 使用**：目标前部出现 `*(T*)p`、`reinterpret_cast<T*>(p)`、
  `(T*)(uintptr_t)p`、`p->field` 等等价解引用。

命中后：

1. 通过 `getCPtr`、字段 writer、app 调用点和 native factory 窄检索定位工厂；
2. 工厂及 library/offset 均确认后标为 `OPAQUE_HANDLE`，将其返回值绑定到 consumer；
3. 递归解决工厂参数，且仍受深度预算限制；
4. 工厂缺失或超深度时标为 `UNRESOLVED_HANDLE`，返回 `SKIP`，原因
   `UNRESOLVED_OPAQUE_HANDLE`。

任何情况下都不得将该参数改成 `FUZZ_INPUT`、`UNKNOWN` 或普通 `CONSTANT`。

## 5. 窄检索策略

| 依赖 | 检索目标 |
|---|---|
| `EXPLICIT_OBJECT` | constructors、factories、Builder 入口、app 真实实例化点 |
| `FIELD_READ` | 目标字段的 reader，以及 constructor/setter/直接赋值等 writer 和字段值来源 |
| `IMPLICIT_HANDLE` | `getCPtr`、handle 字段 writer、`new_*`、native reinterpret |
| `IMPLICIT_FIELD_WRITE` | native `Set*Field`、Java 对象生命周期和对应 reader |
| `IMPLICIT_GLOBAL_STATE` | 同库 init/create/open/load、全局变量 writer、app 初始化调用 |
| `IMPLICIT_RETURN_VALUE` | 兼容 native producer、调用点赋值和返回值保存位置 |
| `CALL_ARGUMENT` | 直接实参、上层调用者、常量集合、字段来源 |

每轮排除已看过且无新信息的证据锚点；扩展一个局部上下文层级优于扩大关键词到全包。
候选调用必须有明确贡献，析构/释放调用通常不是 provider，除非源码证明目标要求该状态。

## 6. Resolution 与 Review

### CheckLocalResolution

- 当前 `S_m/A_m` 是否覆盖已识别依赖；
- 新调用的每个参数是否有来源，未完成者加入下一轮 `D_m`；
- producer 是否在 consumer 前，字段/全局状态的 write 是否在 read 前；
- 是否出现循环、重复初始化、相互矛盾 binding；
- 是否误将 handle 当作整数，或将 `UNKNOWN` 当作完成。

复杂对象的 `CONSTRUCT` 交接至少包含类名、目标参数、native-read fields、已知实例化证据和源码锚点。
本阶段 OK 不代表该对象已经通过 Algorithm 2 的编译或设备验证。

### GlobalReview

1. 整个序列的数据流与副作用顺序成立；
2. 所有 implicit dependencies 有具体 native provider，无 `OPEN/BLOCKED`；
3. 所有参数有允许标签，所有 `NATIVE_RETURN/OPAQUE_HANDLE` 递归闭包；
4. `role/source/binding` 与实际证据一致；
5. 前置调用表和构造需求可被后续技能直接消费。

任一失败即拒绝，不输出“基本完整”的成功结果。

## 7. 预算与失败

- 最多 20 个 resolution rounds；
- 前置 native 链最多 3 层；
- 同一依赖连续 2 轮没有新证据即标 `BLOCKED`，换候选或停止；
- 每轮限制检索结果数，优先高相关局部证据，不以增加轮数掩盖无信息查询；
- 预算耗尽且 `D_m` 非空返回 `FAIL`；handle 硬门禁按主文档返回 `SKIP`。

失败产物必须记录未满足依赖、已尝试查询、缺失 provider 和下一修复入口，不得写入 harness-memory。
