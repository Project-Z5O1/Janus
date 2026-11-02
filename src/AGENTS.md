# Omen — Agentic JNI Fuzzing

Omen is the engineering implementation of the Janus architecture for generating JNI-aware
Android native fuzzing harnesses. It uses repository-local `AGENTS.md` instructions and
standard Agent Skills; no client-specific agent framework is required.

## Architecture

```text
APK
  -> Cross-Language Parser
     Java + Smali + native pseudocode + JNI Binding Table
  -> Harness Synthesizer
     Structure-Aware Retriever
     Algorithm 1: Dependency-Driven Call Sequence Extraction
     Algorithm 2: Recursive Argument Object Construction
     Harness Memory (verified components shared across JNI methods)
  -> Feedback-Driven Harness Refiner
     fixed template -> build -> semantic review -> runtime -> coverage
     failures return to the responsible synthesizer component
  -> AFL++ Frida Mode fuzzing
```

Target selection and priority scoring are engineering gates after parsing. They order eligible
work but are not additional conceptual stages in the paper architecture.

## Canonical skills

The only skill source is `.agents/skills/`. OpenCode and ZCode discover it directly; Claude Code
uses `.claude/skills`, which links to the same directory.

| Skill | Responsibility |
|---|---|
| `omen-pipeline` | Serial orchestration, recovery, retries, state transitions |
| `cross-language-parser` | APK preprocessing, JNI binding, target/priority gates |
| `structure-aware-retriever` | LSP + ast-grep + text evidence across Java/Smali/native |
| `dependency-driven-call-sequence` | Algorithm 1 and `call_sequence.md` |
| `recursive-argument-constructor` | Algorithm 2 and `argument_plan.md` / recipes |
| `harness-memory` | Package-global verified call sequences and argument recipes |
| `feedback-driven-harness-refiner` | Template assembly, review, runtime/coverage repair loop |
| `afl-fuzzing` | Real-device AFL++ execution and hard-gate metrics |

Run the full workflow with `/omen-pipeline [apk-path]`. Individual skills may also be invoked
for a single stage. Client command files are thin launchers, not separate workflow definitions.

## Algorithm invariants

### Algorithm 1 — invocation context

Start from unsatisfied explicit and implicit dependencies, not from a guessed caller chain.
Maintain `S_m`, `A_m`, and `D_m` until dependency resolution reaches a fixed point. Every
prerequisite call needs `role`, `source`, and `binding` evidence and must pass local resolution
plus global review. A native pointer/opaque handle must come from a verified factory; it must
never be fuzzed as an integer or silently replaced with `NULL`.

### Algorithm 2 — argument objects

Terminate recursion with direct JNI synthesis or a compatible verified memory entry. Select
constructor/factory paths using real app instantiation and native-read field coverage, recursively
construct subarguments, backtrack on failure, and assemble bottom-up. Only symbol-checked,
independently built, device-smoke-tested recipes may enter Harness Memory. Do not use default
`NULL`, an arbitrary `byte[]` constructor, or residual-field patching to hide an invalid path.

## Cross-stage contracts

```text
target_APK/<package>/
  base.apk
  jni_list.json
  signatures_libraries_offsets.txt
  targets.md / filtered.md / priority.md
  summary.json
  memory/
  src/{java,smali,native}/
  <jni>/
    call_sequence.md
    argument_plan.md
    object_recipes/
    fuzz.c / afl.js / build/
    harness_review.md
    fuzzing_metrics.json
```

- Algorithm 1 owns call order and dependency bindings; Algorithm 2 owns argument construction.
- The refiner consumes both artifacts and fills only published `GENERATOR` template slots.
- `summary.json` schema v2 is the only runtime state. Markdown manifests are audit evidence.
- Only `.agents/skills/omen-pipeline/scripts/omen_state.py` changes runtime state.
- Only `.agents/skills/harness-memory/scripts/memory_store.py` changes the Memory index.
- `jenv.h` and `libjenv.so` are fixed assets and must not be modified.

## Scheduling and feedback

Parsing may parallelize JADX, apktool, and per-library Ghidra jobs. All per-JNI synthesis,
refinement, and fuzzing work is strictly serial. A single Android device runs at most one Omen
fuzzing task at a time.

Failures are routed by evidence:

- template, generated C, or link slot: `harness`
- missing prerequisite, call order, global state, or handle factory: `call-sequence`
- argument type/state, input constraint, or shallow coverage: `argument-construction`
- unavailable device/toolchain/AFL dependency: `environment`

A repaired upstream component invalidates all downstream validation. Environment failures stop
automatic retries until explicitly resumed. Each synthesis/refinement/fuzzing stage has at most
three consumed retries.

## Build and device rules

Run commands from the repository root. Use environment configuration instead of host-specific
absolute paths:

- `OMEN_ROOT`: optional repository override for helpers
- `ANDROID_NDK_HOME` or `ANDROID_NDK_ROOT`: Android NDK installation
- `OMEN_DEVICE_ID`: Android serial when multiple devices are connected
- `OMEN_AFL_DIR`: optional directory containing `afl-fuzz` and `afl-frida-trace.so`

Use `python3 .agents/skills/omen-pipeline/scripts/preflight.py --stage <stage>` before work that
needs external tools. The repository contains source templates, but AFL++/Frida device binaries
are an external runtime dependency unless explicitly installed under `assets/afl-binaries/`.

For harness target addresses, always use `dlopen` + `get_library_base` + relative offset. Never
resolve target JNI functions with `dlsym`; stripped and dynamically registered methods may not
export symbols. Ghidra's image base is not a runtime offset.

Use `/data/local/tmp/omen/<jni>/` for a target and the namespaced recipe helper for constructor
smoke tests. Clean only directories owned by the current task. Pull logs, metrics, and crash
samples before cleanup. Do not modify harness code merely to suppress a real target crash.

## Compatibility and portability

Shared skills use only `name` and `description` frontmatter and describe generic read, search,
LSP, shell, and file capabilities. Client-specific model selection, permissions, subagent syntax,
and credentials do not belong in the shared workflow. Keep API keys in client or environment
configuration, never in repository files.
