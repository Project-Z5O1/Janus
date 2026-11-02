# JNI Call-Sequence Analysis

Analyze JNI target eligibility, priority, and parameter-preparation call sequences from local source under `target_APK/<package>/src/`.

## Responsibilities

This skill has three task-selected modes:

1. **Target filtering mode**: generate `phase2_targets.md` and `phase2_filtered.md`.
2. **Priority scoring mode**: generate `phase2_priority.md`.
3. **Dependency-driven call-sequence mode**: analyze one JNI function and produce `callingsequence.md`.

Modes 1 and 2 are package-level gates executed directly by the Agent with tools such as ast-grep, grep_search, and LSP. Do not rely on external helper scripts for the actual per-function analysis. Mode 3 handles only the assigned JNI and does not manage package-level loops or retry policy.

Mode 3 returns only `callingsequence.md` and analysis evidence. Do not return a `summary.json` patch, `oldString/newString`, or edit directives. Runtime Phase 2 state is written only by the orchestrator through the summary helper.

## Analysis Engine

- Use `ast-grep` for structured Java code search.
- Use LSP references, definitions, and symbols for cross-checking.
- Fall back to `src/smali` with the Smali LSP when Java decompilation is missing or distorted.
- When Smali hits `.method ... native` or an `invoke-*` native call site, use Smali LSP definition/hover to jump into `src/native/<abi>/<lib>.c` and record JNI name, library, and offset.
- Do not use `jadx-mcp`.
- Do not batch-infer call chains or batch-generate `callingsequence.md` with scripts.

## Prerequisites

1. `target_APK/<package_name>/jni_list.json` or `signatures_libraries_offsets.txt`.
2. `target_APK/<package_name>/src/java` (JADX output; Java root may be `src/java/` or `src/java/sources/`).
3. `target_APK/<package_name>/src/smali` (apktool disassembly).
4. `target_APK/<package_name>/src/native` (Ghidra decompiled C source).

## Mode 1: Target Filtering

Read `jni_list.json` in original discovery order. For each JNI function:

1. Resolve a Java native declaration. Prefer `src/java`; fall back to `.method ... native` in `src/smali` when Java is missing or distorted.
2. Verify at least one Smali `invoke-*` call site references the native method. If `src/smali` is unavailable, skip this call-site check.
3. Record concrete call-site evidence as `smali path:line` plus the full invoke instruction, keeping the first three entries and using `... and N more` for overflow.
4. Do not filter zero-explicit-parameter native methods; they may operate on object state set up by prerequisite calls.

Write eligible functions to `phase2_targets.md` and filtered functions to `phase2_filtered.md` using normalized reasons:

| reason | meaning |
| --- | --- |
| `FILTERED_NON_JAVA_NATIVE` | Cannot map to a Java/Smali native declaration. |
| `FILTERED_NO_JAVA_CALLSITE` | Declaration exists, but no Smali invoke references the method. |

Update `summary.json` once during the gate reduce: targets remain pending, filtered functions become SKIP for downstream phases.

## Mode 2: Priority Scoring

Score only functions already present in `phase2_targets.md`. The primary evidence source is Ghidra-decompiled native C under `src/native/`; Java, Smali, and `jni_list.json` are supplementary.

For each target, locate the native function by library and offset, read the complete implementation and directly called internal helpers, then score these dimensions:

| dimension | evidence |
| --- | --- |
| `complexity` | Branches, loops, nesting, and control-flow density. |
| `pointer_ops` | Pointer arithmetic, memory operations, casts, and raw buffer access. |
| `callee_depth` | Fan-out into internal helpers and layered processing. |
| `vuln_surface` | Bounds, resource lifetime, integer, handle, and state-machine risk patterns. |

Sort by descending score and break ties by the original `order` from `phase2_targets.md`. Write `phase2_priority.md` and copy `priority_score`, `priority_tier`, `priority_rank`, and `priority_reason` into `summary.json` during one reduce write. This ranking estimates likely fuzzing value; do not present it as guaranteed crash prediction.

## Mode 3: Dependency-Driven Call Sequence

Core principle: the call sequence is a byproduct of resolving how each target JNI parameter should be prepared. The driving question is “which dependencies are still unsatisfied for executing the target JNI?”, not “who calls this method?”.

### Step 1: Prepare

1. Read `jni_list.json` and locate the target JNI signature.
2. Confirm the target belongs to `phase2_targets.md` when that manifest exists.
3. Analyze `src/java` recursively, with `src/java/sources/` compatibility; supplement from `src/smali` when needed.
4. Locate the target native implementation in `src/native/<abi>/<lib>.c` by library and offset.

### Step 2: Locate Declarations, Call Sites, and Native Code

Use ast-grep and LSP to locate the Java native declaration. Search direct Smali call sites with full class name and method name. Treat these call sites as dependency evidence entry points, not as the final call sequence by themselves.

### Step 2.5: Opaque Handle Detection

Before normal parameter resolution, identify `jlong`/`jint` values that are actually native object pointers. Any one evidence channel is sufficient:

- **SWIG triple**: `swigCPtr`, `static long getCPtr(Class)`, and native factory/destructor pairs such as `new_<Class>` / `delete_<Class>`.
- **Java-side handle chain**: `getCPtr`, `swigCPtr`, `mNativePtr`, `m*Ptr`, `m*Handle`, or `m*Ctx` flows into the parameter.
- **Native-side reinterpret**: early target native code casts or dereferences the parameter as a pointer.

If a factory is found, label the parameter `OPAQUE_HANDLE(class, factory_jni, lib, offset)`. If no factory is found, label it `UNRESOLVED_HANDLE(class)`. Never downgrade an opaque handle to `FUZZ_INPUT`.

### Step 3: Build the Open Dependency Set

For each explicit target parameter and native state dependency, create a requirement entry with stable id, consumer, need, evidence, status, and provider. Include Java call-site arguments, object fields read by native code, global initialization flags, native-return value flows, and opaque-handle factories.

### Step 4: Close Dependencies Iteratively

Loop until the dependency set is closed or limits are exhausted:

1. Generate narrow queries for each open requirement.
2. Execute ast-grep, LSP, Smali LSP, and native-source searches.
3. Convert evidence into candidate producer calls or construction paths.
4. Insert accepted producers before their consumers and annotate source anchors.
5. Recheck local closure; prerequisite calls may introduce new parameter requirements.

Every prerequisite call must include structured metadata: id, producer, consumer, call, reason, source, and binding.

### Step 5: Resolve Parameter Labels

Each explicit target parameter must end with exactly one terminal label:

| label | meaning | harness handling |
| --- | --- | --- |
| `FUZZ_INPUT` | Directly from fuzz buffer. | Read from buffer by type. |
| `CONSTANT(value)` | Same value at all call sites. | Hardcode. |
| `STRING` | String from fuzz data. | `NewStringUTF`. |
| `SIMPLE_ARRAY(element_type)` | Primitive array. | `NewXxxArray` + `SetXxxArrayRegion`. |
| `CONSTRUCT(class)` | Java object construction. | Use object recipe flow. |
| `NATIVE_RETURN(func, lib, offset)` | Return value of another native function. | Execute prerequisite call and bind return value. |
| `OPAQUE_HANDLE(class, factory_jni, lib, offset)` | Native object pointer hidden in integer type. | Execute prerequisite factory; never fuzz as integer. |
| `UNRESOLVED_HANDLE(class)` | Opaque handle detected but no factory found. | Verdict is insufficient; Phase 3 skips. |
| `FIELD_READ(class, field)` | Object field whose writer is known. | Track writer and possibly prerequisite call. |
| `UNKNOWN` | Cannot trace safely. | Pass NULL or fuzz input and annotate residual risk. |

If a parameter flows from another native call, recursively resolve that prerequisite call's parameters with the same rules, to a maximum depth of 3.

### Step 6: Closure Check

Analysis is sufficient only when all parameters have terminal labels, all `NATIVE_RETURN` and `OPAQUE_HANDLE` prerequisite chains are recursively resolved, and any `FIELD_READ` writer is reproducible. `PARAM_PASSTHROUGH` or `UNRESOLVED_HANDLE` means the analysis is incomplete and the verdict is insufficient.

`UNKNOWN` is allowed only for residual call-argument uncertainty without strong producer evidence. It must not be used to bypass opaque handles, native field writes, global state, or native-return dependencies.

### Output Format

`callingsequence.md` must be stable and directly consumable by `jni-harness`. Include:

1. Target function summary.
2. Opaque-handle analysis.
3. Parameter resolution table.
4. Structured prerequisite call table with columns `step`, `jni_name`, `library`, `offset`, `parameter source`, and `return value usage`.
5. Object recipe references, if any.
6. Source anchors and evidence.
7. Verdict: `sufficient` or `insufficient` with explicit gaps.
