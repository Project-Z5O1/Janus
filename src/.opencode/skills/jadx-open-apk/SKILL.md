# APK Source Preprocessing

Decompile APK source inputs for later JNI analysis.

## Responsibilities

Generate Java, Smali, and native C source trees under `target_APK/<package>/src/`. Phase 1 may parallelize JADX, apktool, and per-library Ghidra exports as long as every Ghidra job has a unique project/work directory.

## Outputs

- `src/java` from JADX
- `src/smali` from apktool
- `src/native/<abi>/<lib>.c` from Ghidra headless export

## Rules

- Use JADX with `--no-res --show-bad-code`.
- Use apktool with `--no-res`.
- Export native code with `ExportSoDecomp.java` and preserve absolute Ghidra address, relative library offset, and image base comments.
- Do not start Phase 2 analysis from this skill.
