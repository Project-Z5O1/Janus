# /jni-pipeline — JNI Full Workflow Automation

Automatically scan **all APKs** under `apks/` and process each one through the full workflow: JNI discovery -> source preprocessing -> eligibility filtering -> value ranking -> call-sequence analysis -> harness generation -> AFL++ fuzzing.

## Usage

### Fully Automatic (Recommended)
```
ulw /ralph-loop "Execute /jni-pipeline" --max-iterations  1000
```
No arguments are required. It scans `apks/*.apk` automatically and skips completed APKs/JNI entries according to the recovery mechanism.

### Manual Mode
```
/jni-pipeline
```
No arguments are required here either, but after a context interruption you must enter it again manually.

### Specify a Single APK (Optional)
```
/jni-pipeline apks/some.apk
```
Process only the specified APK and skip the rest.

## Core Loop

You are a very thin orchestrator. You **only** schedule work and write state back; you do not perform any concrete analysis or generation work.

### Orchestration Rules

- This is an **Agent-orchestrated workflow**. The global scheduling policy is constrained by this Markdown file and `AGENTS.md`; do not introduce an additional executable scheduler.
- `summary_state.py` is responsible Only for atomic `next / start / finish` operations for Phase 2/3/3.5/4. It does not perform global cross-phase scheduling; even in serial mode it keeps an exclusive file lock and atomic writes to prevent partial writes if the main process crashes.
- Step 1 / 1.5 / 1.75 still proceed in package-level gate order; each package gate reduce may have only one writer.
- **Step 1.75 all phases after (Phase 2 / 3 / 3.5 / 4) must be strictly serial**: only one task() may run at a time; do not claim/dispatch the next task() until the previous task() returns and helper finish completes.
- Serial execution reduces state inconsistency and recovery cost introduced by concurrency; do not break this constraint for throughput.
- **Phase 4 must keep a single serial slot in a single-device setup**: at most one `fuzzing-test` task() may occupy the device at a time (this constraint is naturally satisfied in serial mode)

### Scheduling Loop

Serial scheduling: each round claims and dispatches **one** task(), waits for it to complete and for finish to write back to the summary, then starts the next round.

```
while true:
  1. First fill in any missing gate / manifest for the current APK (Step 1 / 1.5 / 1.75)
  2. Select the single next task by the following priority order (only one per round):
     a. Phase 4 eligible (phase3_review==APPROVED and phase4_fuzz claimable) -> claim the highest-priority JNI
     b. Otherwise Phase 3.5 eligible (phase3_build==OK and phase3_review claimable) -> claim the highest-priority JNI
     c. Otherwise Phase 3 eligible (phase2_callseq==OK and phase3_build claimable) -> claim the highest-priority JNI
     d. Otherwise Phase 2 eligible (phase2_callseq claimable) -> claim the highest-priority JNI
  3. Dispatch the single task() for that JNI and block in the main loop until it returns
  4. task() returns, use summary helper `finish` to write back the terminal state (Phase 4 may write metrics at the same time)
  5. Start the next round (do not dispatch a second task() concurrently)
  6. If no JNI has residual work -> output <promise>DONE</promise>
```

The selection order prioritizes later phases so JNI entries that have already reached Phase 3/4 converge to terminal states quickly, avoiding long-lived in-flight backlog.

### Summary Helper Convention

Phase 2/3/3.5/4 runtime state fields in `summary.json` have only one valid writer:

```bash
python3 .opencode/command/scripts/summary_state.py
```

In serial execution mode, the helper exposes only three operation types (no worker_id / lease / heartbeat anymore):

```bash
# 1. Select the next JNI to process (read-only, priority-sorted)
python3 .opencode/command/scripts/summary_state.py \
  next --package "{package}" --phase phase3

# 2. Enter IN_PROGRESS (debug breadcrumb useful for crash recovery; optional)
python3 .opencode/command/scripts/summary_state.py \
  start --package "{package}" --phase phase3 --jni-name "{jni_name}"

# 3. Write back the terminal state
python3 .opencode/command/scripts/summary_state.py \
  finish --package "{package}" --phase phase3 --jni-name "{jni_name}" \
  --result OK --last-error "" --fix-applied ""

# Phase 4 finish with metrics (optional):
python3 .opencode/command/scripts/summary_state.py \
  finish --package "{package}" --phase phase4 --jni-name "{jni_name}" \
  --result PASS --metrics '{"stability":95.5,"paths_total":12,"bitmap_cvg":0.38,"execs_per_sec":45.2,"saved_crashes":0,"saved_hangs":0}'
```

`finish` stdout includes a next-queue preview in addition to the terminal state fields, with the following structure:

```json
{
  "ok": true,
  "phase": "phase3",
  "jni_name": "Java_xxx",
  "result": "OK",
  "next": {"phase": "phase3_review", "jni_name": "Java_yyy", "priority_rank": 2,..},
  "remaining": {"phase4": 0, "phase3_review": 1, "phase3": 4, "phase2": 0}
}
```

Hard Scheduling Constraints (prevent Agent self-interruption):

- After every `finish`, you **must** immediately dispatch the next task() in the same response based on the returned `next` field; do not run a separate `next` again.
- Only when `finish` returns `"all_done": true` (that is, `next == null` and all `remaining` counts are 0) may you output `<promise>DONE</promise>`
- `remaining`: it should be impossible for any remaining count to be > 0 while `next` is null; if that happens, treat it as state corruption and rerun `next` to verify instead of terminating.

Crash-recovery semantics: If the previous round dies in the middle of task(), leaving an `IN_PROGRESS` entry, the next round automatically treats it as selectable again; no time comparison or lease-expiration check is needed.

If you need to write back Phase 2 / 3.5 / 4, replace `--phase` / `--result` with the corresponding values.
If you start constructing `oldString/newString`, an `edit` patch, or a single-JNI block replacement for `summary.json`, you have drifted away from the workflow;
Phase 2/3/3.5/4 runtime updates must immediately switch back to the helper.

### Device Directory Isolation

Each JNI function uses an isolated device work directory `/data/local/tmp/omen/{jni_func}/`, 
therefore Phase 3 standalone verification and Phase 4 file materialization are isolated from each other.
But this **does not mean** Phase 4 can run concurrently on a single device; `adb root` / `wait-for-device` / `logcat -c`
are still device-wide global operations.

### Forbidden Actions

- **Do not** directly perform code analysis, harness generation, compilation, or device operations in the main context
- **Do not** maintain progress.md / findings.md / task_plan.md (generate them only during final wrap-up)
- **Do not** stuff 500 lines of instructions into the prompt (instructions live in Skills, and task() loads them automatically via `load_skills`)
- **Do not** bypass the helper and directly rewrite Phase 2/3/3.5/4 runtime state in `summary.json`; use only `next / start / finish`
- **Do not** use `edit` / replace / `oldString` / `newString` text replacement on a single JNI entry in `summary.json` to update
  `phase2_callseq`, `phase3_build`, `phase3_review`, `phase4_fuzz`, retries or error fields

## Step 0: Initialization

```
If the user specified an apk_path argument:
  apk_list = [apk_path]
Otherwise:
  apk_list = ls apks/*.apk

If apk_list is empty -> report an error "apks/ No APK files found under apks/"

For each APK:
  package = extract the package name with aapt dump badging (or read an existing summary.json)
  If target_APK/<package>/summary.json exists and is complete -> resume
  Otherwise -> need to start from Phase 1 start

Process APKs one by one; finish all phases for one APK before moving to the next.
```

## Step 1: Phase 1 — JNI Discovery + Source Preprocessing

For APKs that have not completed Phase 1:

```
task(
  category="jni-mechanical",
  load_skills=["jni-analysis", "jadx-open-apk"],
  prompt="""
TASK: For APK {apk_path} (package {package}) perform JNI function discovery and source preprocessing.

EXPECTED OUTCOME:
1. target_APK/{package}/jni_list.json — JNI function metadata
2. target_APK/{package}/signatures_libraries_offsets.txt — raw signatures
3. target_APK/{package}/src/java — Java decompiled source container (the actual source root may be `src/java/` itself, or it may be `src/java/sources/`)
4. target_APK/{package}/src/smali — Smali disassembled source
5. target_APK/{package}/src/native — Ghidra decompiled C source
6. target_APK/{package}/summary.json — initialize all functions as PENDING

MUST DO:
- Call the jni-analysis skill and run via CLI `python /Users/syc/Person/Work/Omen/mcptool/jniinfo.py extract..` to extract JNI signatures
- jniinfo.py device-side work directory must use a unique root for each extraction and must not reuse the shared `/data/local/tmp/offsetFinding/` workspace
- Call the parallel preprocessing entrypoint in the jadx-open-apk skill to generate all three source forms.
- Within Phase 1, run in parallel `jadx`, `apktool`, and run Ghidra Headless in parallel per `.so`.
- Ghidra parallelism must use unique project/work directories to avoid different `.so` jobs overwriting each other.
- Initialize summary.json, default each function to phase2_callseq=PENDING, phase3_build=PENDING, phase3_review=PENDING, phase4_fuzz=PENDING
- `summary.json.functions[]` canonical identifier field must be `name`, with the value equal to the corresponding JNI name; do not write only `jni_name`
- `summary.json` must directly follow the runtime schema below; do not rename or omit runtime fields yourself.
- Initialize priority fields as empty:phase2_target_order=null, priority_score=null, priority_tier=null, priority_rank=null, priority_reason=""
- verify all output directories are non-empty.

MUST NOT DO:
- Do not enter Phase 2 analysis.
- Do not modify jenv.h or libjenv.so.

CONTEXT:
- apk_path: {apk_path}
- package: {package}
- device_id: {device_id}
- output_dir: target_APK
"""
)
```

task() returns, verify that summary.json has been created.

## Step 1.5: Phase 2 Target Filtering

For APKs that already have jni_list.json but have not yet generated phase2_targets.md:

```
task(
  category="jni-mechanical",
  load_skills=["jni-callsequence"],
  prompt="""
TASK: Generate the Phase 2 target-filtering manifests for package {package}.

EXPECTED OUTCOME:
1. target_APK/{package}/phase2_targets.md — manifest of functions eligible for Phase 2
2. target_APK/{package}/phase2_filtered.md — excluded functions and reasons

MUST DO:
- Read target_APK/{package}/jni_list.json
- Prefer calling jni-callsequence Step 1.5 gate helper
- Run the map stage in parallel for each function: recursively locate native declarations in src/java (compatible with `src/java/sources/`), falling back to src/smali when insufficient.
- No declaration can be mapped -> FILTERED_NON_JAVA_NATIVE
- Smali has no invoke call site -> FILTERED_NO_JAVA_CALLSITE
- 0 parameter functions are not filtered and still enter targets.
- Generate once during the reduce stage `phase2_targets.md` / `phase2_filtered.md`
- Write back once during the reduce stage `summary.json`:mark filtered functions as phase2_callseq=SKIP, phase3_build=SKIP, phase4_fuzz=SKIP
- Step 1.5 / 1.75 are package-level gates and do not use `summary_state.py` helper; write atomically directly to `summary.json` (read -> modify -> write)
  Ensure only one writer exists at a time (one package gate reduce may have only one writer).

MUST NOT DO:
- Do not start call-sequence analysis.
- Do not process functions outside this APK.

CONTEXT:
- package: {package}
- jni_list.json: target_APK/{package}/jni_list.json
- src_dir: target_APK/{package}/src
"""
)
```

After task() returns:
- verify `phase2_targets.md` / `phase2_filtered.md` has been generated
- verify `summary.json.phase2_target_order` and filtering states have been written back by the reduce stage.

## Step 1.75: Phase 2 Priority Evaluation

For APKs that already have `phase2_targets.md` but have not yet generated `phase2_priority.md`:

```
task(
  category="jni-mechanical",
  load_skills=["jni-callsequence"],
  prompt="""
TASK: Generate a static priority ranking for Phase 2 candidate JNI entries in package {package}.

EXPECTED OUTCOME:
1. target_APK/{package}/phase2_priority.md — ranking results for candidate JNI entries

MUST DO:
- Prefer calling the jni-callsequence Step 1.75 gate helper
- Read and score only `phase2_targets.md` functions
- Run map-stage static scoring for each target in parallel.
- Combine jni_list.json, src/java (recursive, compatible with `src/java/sources/`), src/smali, and src/native for static heuristic scoring.
- For each target, output:priority_score, priority_tier, priority_rank, priority_reason
- Scoring dimensions must cover at least:crash_signal, harnessability, execution_cost
- Tie scores are ordered by phase2_targets.md original order.
- Conservatively reduce the score when evidence is insufficient; do not describe this step as "predicting guaranteed crashes".
- Generate once during the reduce stage `phase2_priority.md` and write back `summary.json.priority_*`
- Step 1.75 are package-level gates and do not use `summary_state.py` helper; write atomically directly to `summary.json` (read -> modify -> write)

MUST NOT DO:
- Do not rerun eligibility filtering.
- Do not start call-sequence analysis.
- Do not generate a harness or trigger device operations.
- Do not run concurrently with Step 1.5; a package gate reduce may have only one `summary.json` writer

CONTEXT:
- package: {package}
- jni_list.json: target_APK/{package}/jni_list.json
- phase2_targets: target_APK/{package}/phase2_targets.md
- src_dir: target_APK/{package}/src
"""
)
```

After task() returns:
- verify `phase2_priority.md` has been generated
- verify `priority_score`, `priority_tier`, `priority_rank`, `priority_reason` have been written back to `summary.json`
- If a target lacks a priority result, keep `priority_rank=null`, and treat Step 1.75 as incomplete after this round.

## Step 2: Phase 2 — Call-Sequence Analysis (one JNI at a time, serial)

Use helper `next --phase phase2` to select the highest-priority eligible JNI from `phase2_targets.md`.
This phase selects one JNI per round and waits for task() to return; do not dispatch concurrently.
helper treats the following states as selectable again:
- `phase2_callseq == "PENDING"`
- `phase2_callseq == "IN_PROGRESS"` (left over from a previous task() crash; reclaim directly)

```
task(
  category="jni-deep",
  load_skills=["jni-callsequence"],
  prompt="""
TASK: For JNI function {jni_name} perform call-sequence analysis.

EXPECTED OUTCOME: target_APK/{package}/{jni_name}/callingsequence.md

MUST DO:
- Use ast-grep + LSP to recursively analyze src/java (compatible with `src/java/sources/`), falling back to src/smali when insufficient.
- Trace call paths bottom-up until parameter construction information is sufficient.
- Every parameter must have a clear source and source-code anchor.
- Use the sufficiency criteria in the SKILL (5 conditions).
- After completion, record the verdict in callingsequence.md (sufficient/insufficient)

MUST NOT DO:
- Do not process other JNI functions.
- Do not use jadx-mcp.
- Do not batch-generate call chains with scripts.
- Do not directly edit Phase 2/3/4 runtime fields in `summary.json`; return only the callingsequence result to the orchestrator.

CONTEXT:
- package: {package}
- jni_name: {jni_name}
- jni_list.json info for this function in: {jni_info_json}
- work_dir: target_APK/{package}
- src_dir: target_APK/{package}/src
"""
)
```

After task() returns:
- If callingsequence.md is generated -> use helper `finish --phase phase2 --result OK`
- If it fails or cannot be traced -> use helper `finish --phase phase2 --result SKIP`

## Step 3: Phase 3 — Harness Generation and Compilation (one JNI at a time, serial)

Use helper `next --phase phase3` to select the highest-priority eligible Phase 3 target among JNI entries with `phase2_callseq == "OK"`.
This phase selects one JNI per round and waits for task() to return; do not dispatch concurrently.
helper treats the following states as selectable again:
- `phase3_build == "PENDING"`
- `phase3_build == "IN_PROGRESS"` (left over from a previous crash)
- `phase3_build == "FAIL"` and `phase3_retries < 3`

```
task(
  category="jni-deep",
  load_skills=["jni-harness"],
  prompt="""
TASK: Generate the harness for JNI function {jni_name}, then compile and verify it.

EXPECTED OUTCOME:
1. target_APK/{package}/{jni_name}/fuzz.c — harness source
2. target_APK/{package}/{jni_name}/afl.js — Frida configuration
3. target_APK/{package}/{jni_name}/build/fuzz — compiled artifact
4. target_APK/{package}/{jni_name}/seeds/seed — initial corpus
5. standalone device verification passes

MUST DO:
- Read callingsequence.md to understand the call chain
- Read jni_list.json to obtain the function signature, library name, and offset
- Generate fuzz.c (5 GENERATOR regions) + afl.js + CMakeLists.txt
- fuzz.c JNI function types in fuzz.c must use `typedef (*function_N)`, global variables must be direct function pointers; do not generate `function_N* targetFunctionPtrN`
- If the harness uses `dlopen` / `RTLD_NOW` / `dlerror`, must include `#include <dlfcn.h>`
- For COMPLEX parameters: generate construct.c under object_recipes/ and compile/verify it independently.
- recipe device verification directories must be generated through `recipe_device_dir.py` to create package/jni/class namespaced paths.
- After final fuzz.c assembly, perform cross-compilation (build.sh)
- Push to the device and run standalone verification (echo 'AAAA' |./fuzz)
- All device-side `.so` files must be flattened under `DEVICE_DIR/lib/*.so`; Do not push the entire `lib/` directory as `DEVICE_DIR/lib/lib/*.so`
- standalone verification must explicitly use absolute `DEVICE_DIR/lib`, do not rely only on `$ORIGIN/lib`
- Retry at most 3 compile/verification failures.

MUST NOT DO:
- Do not modify jenv.h or libjenv.so.
- Do not resolve function addresses with dlsym (must use get_library_base + offset).
- Do not skip the object_recipes flow for COMPLEX parameters.
- Do not directly rewrite `phase3_build`, `phase4_fuzz`, retries, or error fields in `summary.json`
- Do not return a `summary.json` patch, `oldString/newString`, or edit directives; return only the result to the orchestrator.

CONTEXT:
- package: {package}
- jni_name: {jni_name}
- jni_info: {jni_info_json}
- callingsequence: target_APK/{package}/{jni_name}/callingsequence.md
- assets_dir: /Users/syc/Person/Work/Omen/assets
- device_id: {device_id}
"""
)
```

After task() returns:
- compile + standalone passes -> use helper `finish --phase phase3 --result OK`
- failure -> use helper `finish --phase phase3 --result FAIL`, also writing `phase3_last_error`
- `phase3_retries < 3` the helper may reclaim it in the next round.
- `phase3_retries >= 3` treat Phase 3 as terminal FAIL and set `phase3_review` to `SKIP`, `phase4_fuzz` to `SKIP`

## Step 3.5: Phase 3.5 — Harness Quality Review (one JNI at a time, serial)

Phase 3 After Phase 3 compile + standalone passes and before submitting to device AFL++ verification, perform semantic quality review of the harness,
ensuring the generated harness can effectively simulate the real call path.
This phase claims one JNI per round and waits for task() to return; do not dispatch concurrently.

Use helper `next --phase phase3_review` to select, among JNI entries with `phase3_build == "OK"` and `phase3_review == "PENDING"`,
highest-priority eligible target.
This phase selects one JNI per round and waits for task() to return; do not dispatch concurrently.
helper treats the following states as selectable again:
- `phase3_review == "PENDING"`
- `phase3_review == "IN_PROGRESS"` (left over from a previous crash)

```
task(
  category="jni-deep",
  load_skills=["harness-review"],
  prompt="""
TASK: Perform semantic quality review for the harness of JNI function {jni_name}.

EXPECTED OUTCOME: target_APK/{package}/{jni_name}/harness_review.md (including verdict + 5-dimension scoring)

MUST DO:
- Read callingsequence.md (Phase 2 parameter preparation plan)
- Read fuzz.c (Phase 3 generated harness)
- Read afl.js (Frida configuration)
- Read this function's signature/offset information from jni_list.json
- Read src/native decompiled C code for the target function
- If callingsequence.md references Java callers, read the corresponding files under src/java and src/smali.
- Generate the review report by cross-checking each dimension:
  1. call-path fidelity: whether prerequisite calls are complete and ordered, and whether data flow is correct
  2. parameter parsing adequacy: type mapping, value-domain coverage, and buffer layout
  3. precondition satisfaction: object lifecycle, state dependencies, and parameter constraints
  4. exception-handling distinction: JNI exception-check coverage and cleanup paths
  5. coverage potential assessment: branch reachability and MODULE_WHITELIST correctness
- Provide verdict: APPROVED / NEEDS_REVISION / REJECTED

MUST NOT DO:
- Do not modify fuzz.c / afl.js / CMakeLists.txt.
- Do not trigger compilation or device operations.
- Do not rerun Phase 2 call-sequence analysis.
- Do not directly rewrite summary.json; return only the review result to the orchestrator.

CONTEXT:
- package: {package}
- jni_name: {jni_name}
- jni_info: {jni_info_json}
- work_dir: target_APK/{package}/{jni_name}
- src_dir: target_APK/{package}/src
"""
)
```

After task() returns:
- verdict = `APPROVED` -> use helper `finish --phase phase3_review --result APPROVED`; the JNI becomes eligible for Phase 4
- verdict = `NEEDS_REVISION` -> use helper `finish --phase phase3_review --result NEEDS_REVISION`
  - increment `phase3_review_revision_count`
  - Set `phase3_build` back to `FAIL` (consumes the phase3_retries budget)
  - inject review fix suggestions into the CONTEXT of the next Phase 3 task() prompt.
  - When the next Phase 3 regenerates, it must read `harness_review.md` and fix each item.
- verdict = `REJECTED` -> Handle the same as NEEDS_REVISION, but the orchestrator may consider skipping if the issue cannot be fixed.

## Step 4: Phase 4 — Fuzzing Verification (one JNI at a time, serial)

Use helper `next --phase phase4` to select, among JNI entries with `phase3_build == "OK"` and `phase3_review == "APPROVED"`,
the highest-priority eligible Phase 4 target.
This phase selects one JNI per round and waits for task() to return; in serial mode, a device naturally has at most one Phase 4 worker.
helper treats the following states as selectable again:
- `phase4_fuzz == "PENDING"`
- `phase4_fuzz == "IN_PROGRESS"` (left over from a previous crash)
- `phase4_fuzz == "FAIL"` and `phase4_retries < 3`

```
task(
  category="jni-device",
  load_skills=["fuzzing-test"],
  prompt="""
TASK: Run AFL++ on device to verify the harness for JNI function {jni_name}.

EXPECTED OUTCOME: AFL++ 240 second verification passes (Hard Gate all pass:banner + forkserver + bitmap_cvg>0% + paths_total>=2 + execs_per_sec>0)

MUST DO:
- device preflight gate (adb root, /data/local/tmp writable)
- Clean the device directory.
- Upload: build/fuzz, afl.js, seeds/seed, lib/*.so, base.apk, afl-fuzz
- All device-side `.so` files must be flattened under `DEVICE_DIR/lib/*.so`; Do not produce `DEVICE_DIR/lib/lib/*.so`
- Smoke run: echo 'AAAA' |./fuzz
- smoke run must explicitly use absolute `DEVICE_DIR/lib`, do not rely only on `$ORIGIN/lib`
- AFL 240s verify: afl-fuzz -O -D -G $MAX_INPUT_SIZE -i seeds -o output./fuzz
  (`-G` value is extracted from harness fuzz.c `#define BUFFER_SIZE` extract)
- After AFL exits, collect fuzzer_stats, check Hard Gate metrics (bitmap_cvg>0%, paths_total>=2, execs_per_sec>0)
- Record phase4_metrics (stability, paths_total, bitmap_cvg, execs_per_sec, saved_crashes, saved_hangs)
- Append complete logs to target_APK/{package}/logs/{jni_name}.log
- On failure, use the release profile first, then switch to the The debug profile for troubleshooting.
- After successful verification, generate one-click fuzzing.sh and fuzzing-debug.sh scripts according to fuzzing-test SKILL Section 9.
- Clean the device after verification.

MUST NOT DO:
- Do not modify fuzz.c or recompile.
- Do not skip smoke run and enter AFL directly.
- Do not use the The debug profile as The first attempt.
- Do not directly rewrite `phase4_fuzz`, retries, or error fields in `summary.json`
- Do not return a `summary.json` patch, `oldString/newString`, or edit directives; return only the result to the orchestrator.

CONTEXT:
- package: {package}
- jni_name: {jni_name}
- work_dir: target_APK/{package}/{jni_name}
- device_id: {device_id}
- afl_binary: /Users/syc/Person/Work/Omen/assets/afl-binaries/afl-fuzz
"""
)
```

After task() returns:
- AFL verification passes (all Hard Gate checks pass) -> use helper `finish --phase phase4 --result PASS --metrics '{"stability":..,"paths_total":..,"bitmap_cvg":..,"execs_per_sec":..,"saved_crashes":..,"saved_hangs":..}'`
- failure -> use helper `finish --phase phase4 --result FAIL --last-error ".." --metrics '..'` (metrics if collected)
- `phase4_retries < 3` the helper may reclaim it in the next round.

## Step 5: Wrap-Up

When all JNI functions have none of the following states:
- Any phase is still `PENDING`
- Any phase is still `IN_PROGRESS`
- `phase3_build == "FAIL"` and `phase3_retries < 3`
- `phase3_review` is still `PENDING` or `IN_PROGRESS`
- `phase4_fuzz == "FAIL"` and `phase4_retries < 3`

1. best-effort Clean up Phase 1 temporary directories; no match must succeed silently:
   `find "target_APK/{package}" -maxdepth 1 -type d -name '.phase1_tmp.*' -exec rm -rf {} + 2>/dev/null || true`
   `find "${TMPDIR:-/tmp}" -maxdepth 1 -type d -name 'omen_ghidra_*' -exec rm -rf {} + 2>/dev/null || true`
2. Generate the final report from summary.json (optionally generate progress.md / findings.md).
3. output `<promise>DONE</promise>`

## summary.json Format

Schema Notes:
- runtime canonical JNI identifier field is `name`
- If keeping `jni_name`, for compatibility with old artifacts, it must exactly match `name`; new artifacts must not write only `jni_name`
- In serial mode, no longer write `phaseN_owner` / `phaseN_lease_until` / `phaseN_attempt_id`; the helper automatically prunes these legacy fields when loading an old summary.

```json
{
  "package": "com.example",
  "apk_path": "apks/base.apk",
  "total_jni_functions": 10,
  "functions": [
    {
      "name": "Java_xxx_method",
      "phase2_target_order": 1,
      "priority_score": 62,
      "priority_tier": "HIGH|MEDIUM|LOW|null",
      "priority_rank": 1,
      "priority_reason": "direct byte[] input with low harness cost",
      "phase2_callseq": "PENDING|IN_PROGRESS|OK|SKIP",
      "phase2_started_at": "",
      "phase3_build": "PENDING|IN_PROGRESS|OK|FAIL|SKIP",
      "phase3_started_at": "",
      "phase3_retries": 0,
      "phase3_last_error": "",
      "phase3_fix_applied": "",
      "phase3_review": "PENDING|IN_PROGRESS|APPROVED|NEEDS_REVISION|REJECTED|SKIP",
      "phase3_review_started_at": "",
      "phase3_review_issues": "",
      "phase3_review_revision_count": 0,
      "phase4_fuzz": "PENDING|IN_PROGRESS|PASS|FAIL|SKIP",
      "phase4_started_at": "",
      "phase4_retries": 0,
      "phase4_last_error": "",
      "phase4_fix_applied": "",
      "phase4_metrics": {
        "stability": null,
        "paths_total": null,
        "bitmap_cvg": null,
        "execs_per_sec": null,
        "saved_crashes": null,
        "saved_hangs": null
      },
      "error_log": "logs/xxx.log"
    }
  ]
}
```

## Recovery Mechanism

After interruption, rerun `/jni-pipeline`:

1. Read each `target_APK/<package>/summary.json`
2. If missing `phase2_targets.md` / `phase2_filtered.md`, perform first Step 1.5
3. If missing `phase2_priority.md`, perform first Step 1.75
4. Within the current phase, continue from the highest-priority PENDING JNI.
5. Do not repeat completed work.
6. Existing phase2_targets.md/phase2_filtered.md that matches jni_list.json is reused.
7. Existing phase2_priority.md that matches phase2_targets.md entry count is reused.
8. Existing src/ directory that is non-empty is reused.
9. Existing `harness_review.md` and `phase3_review == "APPROVED"` then reuse the review result.

## Constraints

1. The main context only performs summary helper claim/finish + task() dispatch.
2. Each task() handles exactly one phase for exactly one JNI.
3. **Device Directory Isolation**: each JNI uses an isolated `/data/local/tmp/omen/{jni_func}/` directory without interfering with others.
4. summary.json is the single machine-state source.
5. Do not modify jenv.h / libjenv.so
6. Phase 2/3/3.5/4 scheduling must prefer `priority_rank` eligible JNI with the smallest
7. `phase2_targets.md` is the allow-list, `phase2_priority.md` only orders targets and must not change the eligibility set.
8. Phase 3/4 each allow at most 3 failed retries; Phase 3.5 NEEDS_REVISION/REJECTED consumes the Phase 3 retry budget.
9. Phase 2/3/3.5/4 must be strictly serial:each round may have only one in-flight task(); helper `next/start/finish` still use file locks + atomic writes to prevent partial writes if the main process crashes `summary.json`
10. In a single-device setup, Phase 4 must not dispatch multiple `fuzzing-test` task() (naturally satisfied in serial mode)
11. Phase 4 eligibility requires `phase3_build == "OK"` **and** `phase3_review == "APPROVED"`
12. Phase 4 Hard Gate metrics (bitmap_cvg>0%, paths_total>=2, execs_per_sec>0) must all pass before declaring PASS; stability is recorded only as Soft Metrics
