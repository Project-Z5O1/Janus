# JNI Analysis

Discover JNI functions and native-library offsets for one APK.

## Responsibilities

Run the JNI extraction workflow, produce package-local metadata, and initialize `summary.json`. This skill is Phase 1 only. Do not run Phase 2 analysis, harness generation, review, or fuzzing.

## Inputs

- APK path under `apks/`
- Connected Android device
- Repository tools under `mcptool/` and `retool/`

## Outputs

- `target_APK/<package>/jni_list.json`
- `target_APK/<package>/signatures_libraries_offsets.txt`
- Extracted package libraries
- Initialized `target_APK/<package>/summary.json`

## Rules

- Use a per-run namespaced remote workspace for offset extraction. Do not reuse `/data/local/tmp/offsetFinding/` across concurrent APKs.
- Preserve library name and relative offset for each JNI function.
- Initialize all runtime phases as PENDING and priority fields as null/empty.
- The canonical function key in `summary.json.functions[]` is `name`, equal to the JNI name.
- Do not modify `jenv.h` or `libjenv.so`.
