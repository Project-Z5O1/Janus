---
name: omen-pipeline
description: 编排 Omen/Janus 的完整 APK→JNI harness→AFL++ 工作流；当用户要求批量处理 APK、恢复中断任务、执行完整 JNI fuzzing pipeline 或按优先级收敛所有目标时使用。也负责状态迁移、串行调度、反馈回流和环境阻塞恢复。
---

# Omen Pipeline

你是薄编排器。按技能契约调度并验证产物，不在主流程里重新发明源码分析、对象构造或 harness
生成逻辑。每个阶段只处理一个 JNI；完成状态写回后再选择下一个。

## 输入与技能加载

可选参数为一个 APK 路径；缺省处理 `apks/*.apk`，每次完整收敛一个包再处理下一个。
依次使用以下技能：

1. `cross-language-parser`
2. `dependency-driven-call-sequence`（Algorithm 1）
3. `recursive-argument-constructor`（Algorithm 2）
4. `feedback-driven-harness-refiner`
5. `afl-fuzzing`

`structure-aware-retriever` 与 `harness-memory` 是两个算法和 refiner 的共享能力。
使用客户端原生的技能加载方式；若没有显式加载接口，读取对应 `.agents/skills/<name>/SKILL.md`
后执行。可以使用客户端通用子任务能力隔离上下文，但不得依赖特定 category、模型名或循环插件，
且任何时刻最多一个 per-JNI 子任务。

## 状态 helper

唯一运行态写入口：

```bash
python3 .agents/skills/omen-pipeline/scripts/omen_state.py <command> ...
```

常用命令：

```bash
# Parser 后初始化/合并 jni_list.json
python3 .agents/skills/omen-pipeline/scripts/omen_state.py init --package <package>

# 旧 summary 迁移；先备份，再原子替换
python3 .agents/skills/omen-pipeline/scripts/omen_state.py migrate --package <package>

# target/priority gate 同步
python3 .agents/skills/omen-pipeline/scripts/omen_state.py target \
  --package <package> --jni-name <jni> --eligible true --order <n> \
  --score <n> --tier <HIGH|MEDIUM|LOW> --rank <n> --priority-reason "<evidence>"

# 选择、开始、结束一个阶段
python3 .agents/skills/omen-pipeline/scripts/omen_state.py next --package <package>
python3 .agents/skills/omen-pipeline/scripts/omen_state.py start \
  --package <package> --stage <stage> --jni-name <jni>
python3 .agents/skills/omen-pipeline/scripts/omen_state.py finish \
  --package <package> --stage <stage> --jni-name <jni> --result OK

# 将运行/覆盖失败退回责任组件
python3 .agents/skills/omen-pipeline/scripts/omen_state.py repair \
  --package <package> --jni-name <jni> --from-stage harness \
  --owner argument-construction --category INPUT_CONSTRAINT --error "<evidence>"

# 查看收敛/阻塞状态；环境修复后显式恢复
python3 .agents/skills/omen-pipeline/scripts/omen_state.py status --package <package>
python3 .agents/skills/omen-pipeline/scripts/omen_state.py resume --package <package> --jni-name <jni>
```

阶段名为 `call-sequence`、`argument-construction`、`harness`、`fuzzing`；对应状态字段为
`call_sequence`、`argument_construction`、`harness`、`fuzzing`。完整字段与迁移规则见
[state-schema.md](references/state-schema.md)。

不要用手工文本替换修改 `summary.json`。manifest、日志和阶段文件由对应技能写入。

## 包级启动与恢复

对每个 APK：

1. 从已有 `summary.json` 或 APK badging 确定 package；已有 v2 工作区直接恢复。
2. 运行 `preflight.py --stage parser`；若失败，立即将本包报告为 `ENVIRONMENT` 阻塞并停止后续 Parser、
   gate 和 per-JNI 工作。首次 Parser 前还没有 JNI 状态可写，因此只在包级报告中记录，不伪造某个函数的重试。
3. Parser 产物不完整时执行 `cross-language-parser`。其内部可并行，主流程不得自行另开工具链。
4. 确认 `jni_list.json` 后执行 `init`；旧状态先 `migrate`。
5. 确认 `targets.md/filtered.md/priority.md` 与 binding 集合一致，并通过 `target` 同步 gate。
   零参数 JNI 不因参数数量被过滤。
6. 进入串行调度循环。不要重新扫描 `jni_list.json` 绕过 eligible target 集合。

## 串行调度循环

```text
loop:
  snapshot = omen_state.py status --package <package>
  若 snapshot.all_done: 当前包完成，跳出
  若存在 ENVIRONMENT: 报告缺失依赖并暂停本包，不消耗重试
  next = omen_state.py next --package <package>
  若没有可执行任务但 all_done=false: 按状态报告 terminal failure，不伪称成功
  start(next.stage, next.jni_name)
  按 next.stage 调用对应技能，等待一个结构化结果
  若结果要求上游修复:
    repair(from-stage=next.stage, owner=repair_owner, category, error)
  否则:
    finish(stage=next.stage, result, error/fix/metrics, consume_retry)
```

`next` 按 `fuzzing → harness → argument-construction → call-sequence` 优先收敛后段任务，
同一阶段按 `priority_rank`、`target_order`、JNI 名排序。`IN_PROGRESS` 是崩溃恢复面包屑，
可直接重选；没有 lease 或 heartbeat。

## 反馈与重试

- worker 的 `repair_owner` 为 `call-sequence` 或 `argument-construction` 时，用 `repair` 重置其下游，
  不得继续消费旧 harness。
- `harness` 修复后必须重新通过 compile/link、semantic review、runtime、coverage 四门，才能恢复 fuzzing。
- 有效 `FAIL` 默认消耗责任阶段一次重试；上限三次。`ENVIRONMENT` 不消耗重试且不被自动选择。
- `SKIP` 沿下游传播。上游重新成功前，不得手工把下游 `SKIP` 改成成功。
- fuzzing 发现目标 crash 不等于 harness 失败；先保存样本，再区分目标缺陷与 harness 自身错误。

## 环境检查

按即将执行的阶段调用：

```bash
python3 .agents/skills/omen-pipeline/scripts/preflight.py --stage <parser|argument-construction|harness|fuzzing> \
  --package <package> --jni-name <jni>
```

preflight 只检查环境，不把缺失依赖伪造成阶段 FAIL，也不安装或下载工具。恢复与排障规则见
[recovery.md](references/recovery.md)。

## 完成条件

包完成必须满足：没有 `PENDING`、可重试 `FAIL`、可恢复 `IN_PROGRESS` 或 `ENVIRONMENT`，且所有
eligible JNI 都有终态。输出 package、各阶段计数、metrics/crash 摘要和未解决 terminal failure。
不要输出任何客户端专有的完成令牌；直接以可读总结结束。
