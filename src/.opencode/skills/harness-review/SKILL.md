# Harness Review

Semantically review one generated JNI harness before device fuzzing. The goal is to decide whether the harness faithfully exercises the real APK call path and provides useful fuzzing coverage.

## Scope

Review exactly one JNI target. Read existing artifacts and write `harness_review.md`. Do not modify `fuzz.c`, `afl.js`, `CMakeLists.txt`, source code, or `summary.json`. Do not compile or run device operations.

## Inputs

- `callingsequence.md`
- `fuzz.c`
- `afl.js` and optionally `afl-debug.js`
- `jni_list.json`
- Target native source under `src/native`
- Relevant Java and Smali call sites referenced by Phase 2

## Review Dimensions

Score and explain each dimension:

1. **Call-path fidelity**: prerequisite calls, ordering, receiver object, state setup, and return-value data flow.
2. **Parameter parsing adequacy**: type mapping, value ranges, buffer layout, string and array handling, and input-size limits.
3. **Precondition satisfaction**: object lifecycle, opaque handles, field writers, global initialization, and resource symmetry.
4. **Exception handling and cleanup**: JNI exception checks, local-ref cleanup, native resource cleanup, and failure paths.
5. **Coverage potential**: branch reachability, target library whitelist, coverage reset correctness, and whether inputs can reach meaningful native logic.

## Verdicts

- `APPROVED`: safe to proceed to Phase 4.
- `NEEDS_REVISION`: likely fixable harness issues; consumes Phase 3 retry budget.
- `REJECTED`: severe semantic mismatch or unresolved dependency; orchestrator may skip if not fixable.

## Report Format

`harness_review.md` must include:

- Target summary.
- Files reviewed.
- Dimension scores and findings.
- Concrete line/file evidence.
- Required fixes for NEEDS_REVISION or REJECTED.
- Final verdict.

Keep findings actionable. Prefer exact file paths and line numbers when possible.
