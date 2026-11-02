---
name: cross-language-parser
description: 将 Android APK 解析为 JADX Java、apktool Smali、Ghidra native pseudocode 与 JNI Binding Table。处理 APK 预处理、JNI 签名/偏移提取、静态或动态 native 绑定、Parser 产物校验，以及 targets/filtered/priority 工程 gate 时使用。
---

# Cross-language Parser

实现 Janus Phase 1 Parser：把一个 APK 转成可交叉导航的本地源码树和 JNI Binding Table。
目标筛选与优先级评分是后续工程 gate，不属于论文核心阶段，也不得与 Retriever 或两个核心算法混写。

## 输入

- `apk_path`：待分析 APK。
- `package`：应用包名，同时作为工作区目录名。
- `device_id`：已连接且满足提取器要求的 Android 设备。

工作区固定为仓库根 `target_APK/`；外部工具路径通过环境配置，不维护第二套输出树。

完整 Parser 需要设备解析静态导出和动态注册绑定。设备不可用时可以诊断本地源码导出，但不得把未完成的工作区标为 Parser 成功，也不得猜测偏移。

## 统一入口

所有预处理只通过本技能的脚本入口执行：

```bash
bash .agents/skills/cross-language-parser/scripts/preprocess_apk.sh \
  --apk "apks/<apk_name>.apk" \
  --package "<package>" \
  --device "<device_id>"
```

该入口负责临时目录、内部并行、失败清理和产物校验。不要另建平行的预处理流程；手工命令仅用于诊断脚本故障。

## 执行流程

### 1. 准备工作区与 JNI 原始签名

1. 将输入 APK 固定复制为 `target_APK/<package>/base.apk`。
2. 提取 APK 的 arm64 共享库到 `lib/arm64-v8a/`。
3. 复用现有提取器，不重写签名或设备解析逻辑：

```bash
python3 mcptool/jniinfo.py extract \
  --apk "apks/<apk_name>.apk" \
  --package "<package>" \
  --device "<device_id>" \
  --output-dir target_APK
```

4. 保留 `signatures_libraries_offsets.txt` 作为原始绑定证据；解析失败时保留日志并停止后续 gate。

设备解析同时覆盖静态 `dlsym` 与动态 `RegisterNatives`。每次设备 offset extraction 使用唯一 namespace，具体约束见 [binding.md](references/binding.md)。

### 2. 导出三类源码

在 Parser 内部执行以下工作，且不得把并行扩展到后续串行阶段：

- JADX：递归导出 Java 到 `src/java/`；实际源码根可以是 `src/java/sources/`，消费者必须递归查找。
- apktool：导出 Smali 到 `src/smali/`。
- Ghidra Headless：按 `.so` 导出到 `src/native/arm64-v8a/`；保留函数注释中的 entry、相对 offset 和 image base。

JADX 与 apktool 可并行；Ghidra 可按库有界并行，但每个库使用唯一 project/work 目录。任一输出为空都要明确报告，不能用另一语言源码静默替代整个缺失产物。

### 3. 生成 JNI Binding Table

以设备解析结果为主，生成 `jni_list.json`。每项至少包含：

- `jni_name`、`java_class`、`java_method`
- `params`、`return_type`
- `library`、相对 `offset`
- `binding_kind` 与可审计的 `binding_evidence`

按完整 Java 类、方法、签名区分重载。静态与动态记录冲突时不得任意取一个；回到设备日志和库基址重新核验。字段和地址规则见 [binding.md](references/binding.md)。

### 4. 初始化 summary v2

`jni_list.json` 校验后，调用 `omen-pipeline` 的状态 helper 初始化 `summary.json`：

- `schema_version` 为 `2`，`functions[].name` 等于对应 `jni_name`。
- 分别跟踪 `call_sequence`、`argument_construction`、`harness`、`fuzzing`。
- 新条目保持初始状态，不从旧文件名或反编译成功推断后续阶段成功。

本技能不手工拼接或覆盖 summary 运行态；初始化、gate 同步及后续写回都经过同一个带文件锁和原子写的 helper。

### 5. 执行工程 gate

Parser 产物稳定后，依次执行：

1. Target selection：生成 `targets.md` 与 `filtered.md`。
2. Priority scoring：只对 `targets.md` 的集合作静态排序，生成 `priority.md`。

筛选采用 native declaration 加真实 Java/Smali invoke call site；零参数 JNI 不得过滤。评分只使用 `complexity`、`pointer_ops`、`callee_depth`、`vuln_surface` 四维。执行模板、证据和集合校验见 [target-gates.md](references/target-gates.md)。

## 输出契约

```text
target_APK/<package>/
├── base.apk
├── lib/arm64-v8a/*.so
├── src/java/
├── src/smali/
├── src/native/arm64-v8a/*.c
├── jni_list.json
├── signatures_libraries_offsets.txt
├── summary.json
├── targets.md
├── filtered.md
└── priority.md
```

`summary.json` 使用 v2 schema；其余清单是审计产物，不替代 helper 管理的机器状态。

## 完成检查

- `base.apk` 是输入 APK 的固定副本，arm64 库与 native 导出逐库对应。
- Java、Smali、native 源码均存在且非空；Java 采用递归发现，不假定固定 `sources/` 层级。
- 每个 binding 都能回溯到设备证据，`offset` 是相对库基址的偏移。
- `jni_list.json` 与原始签名条目完成去重核对，重载没有合并。
- 设备临时目录和 Ghidra 临时 project 均已清理，且只清理本次运行拥有的 namespace。
- `targets.md ∪ filtered.md` 等于 binding 目标集，`priority.md` 等于 `targets.md` 集合。
- 两个工程 gate 的结果已通过 helper 同步到 summary v2。

## 边界

- 不生成调用序列、参数构造计划或 fuzz harness。
- 不把 Ghidra image base 当作运行时地址，也不在后续 harness 中沿用设备解析用的 `dlsym`。
- 不修改 `jenv.h`、`libjenv.so` 或已验证的分析产物来掩盖提取失败。
