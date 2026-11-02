# JNI Harness Generation

Generate and verify a native AFL++ Frida harness for one JNI target that already has a `callingsequence.md` parameter-preparation plan.

## Responsibilities

This skill handles exactly one JNI function per task. It consumes Phase 2 output, emits Phase 3 artifacts, builds them, and performs standalone device verification. It returns only the Phase 3 result to the orchestrator. Do not directly edit `summary.json`, and do not return patches or `oldString/newString` directives.

## Inputs

- `target_APK/<package>/jni_list.json`
- `target_APK/<package>/<jni_name>/callingsequence.md`
- `target_APK/<package>/src/native`
- Template assets under `/Users/syc/Person/Work/Omen/assets`
- Android device id for standalone verification

## Outputs

- `target_APK/<package>/<jni_name>/fuzz.c`
- `target_APK/<package>/<jni_name>/afl.js`
- `target_APK/<package>/<jni_name>/afl-debug.js`
- `target_APK/<package>/<jni_name>/CMakeLists.txt`
- `target_APK/<package>/<jni_name>/build/fuzz`
- `target_APK/<package>/<jni_name>/seeds/seed`
- Optional `object_recipes/<ClassName>/` artifacts for Java object construction

## Hard Rules

- Never modify `jenv.h` or `libjenv.so`.
- Never resolve target functions with `dlsym`; always use `get_library_base(<lib>) + offset`.
- JNI typedefs must be direct function-pointer typedefs such as `typedef void (*function_0)(JNIEnv*, jobject, jbyteArray);`.
- Global JNI function variables must be direct function pointers, not pointer-to-pointer declarations.
- If `dlopen`, `RTLD_NOW`, or `dlerror` is used, include `<dlfcn.h>`.
- Device `.so` files must land flat under `DEVICE_DIR/lib/*.so`; never create `DEVICE_DIR/lib/lib/*.so`.
- Standalone verification must explicitly use absolute `DEVICE_DIR/lib` in `LD_LIBRARY_PATH`.
- Retry compile or standalone verification failures at most 3 times inside the task.

## Workflow

1. Read `callingsequence.md` and verify the verdict is sufficient. If it is insufficient, return SKIP.
2. Read `jni_list.json` for target signature, library, and offset.
3. Parse the parameter labels and prerequisite call table.
4. Generate object recipes for `CONSTRUCT(class)` parameters and validate each recipe independently.
5. Assemble `fuzz.c` from the template markers.
6. Generate `afl.js`, `afl-debug.js`, `CMakeLists.txt`, build scripts, libraries, and seed corpus.
7. Cross-compile with `build.sh`.
8. Push to the device and run standalone smoke verification with `echo -n 'AAAA' | ./fuzz`.
9. Return OK or FAIL with a concise error and any fix applied.

## Generator Marker Contract

Replace these markers in `fuzz.c`:

| marker | content |
| --- | --- |
| `GENERATOR_FUNCTIONDEFINTION` | Typedefs for prerequisite and target JNI functions. |
| `GENERATOR_GLOBALFUNCTIONS` | Global function pointers. |
| `GENERATOR_FUNCTIONOFFSETS` | Library base lookup plus offset initialization. |
| `GENERATOR_INPUTPARSING` | Buffer parsing for direct fuzz, constants, strings, arrays, and object recipes. |
| `GENERATOR_CALLINGTARGETFUNCTION` | Prerequisite calls, return-value bindings, coverage reset, and target call. |
| `GENERATOR_TARGETLIBRARIES` | Target library names for CMake and Frida whitelist. |

## Parameter Handling

- `FUZZ_INPUT`: consume bytes by C/JNI type with bounds checks.
- `CONSTANT(value)`: hardcode the value.
- `STRING`: create with `NewStringUTF`; delete local refs.
- `SIMPLE_ARRAY(type)`: create the JNI primitive array and fill it from the buffer.
- `CONSTRUCT(class)`: call the validated object recipe.
- `NATIVE_RETURN`: execute prerequisite JNI in table order and bind the return value.
- `OPAQUE_HANDLE`: execute the factory prerequisite and pass the returned handle; never fuzz it as an integer.
- `FIELD_READ`: reproduce the writer path when available.
- `UNKNOWN`: pass NULL or bounded fuzz input and annotate the risk.

Delete local refs in reverse creation order. Check JNI exceptions and clear or fail according to the generated harness policy.

## Object Recipes

For complex Java objects, create `object_recipes/<ClassName>/construct.c` and `recipe.md`. Validate recipes in a package/JNI/class-scoped device directory derived from `.opencode/skills/jni-harness/scripts/recipe_device_dir.py`. Do not reuse a global recipe directory.

## Coverage Reset

When one or more prerequisite native calls exist, isolate target-only coverage:

- Declare `__omen_afl_area_ptr` and a 64KB bitmap snapshot in `fuzz.c`.
- Have `afl.js` and `afl-debug.js` write AFL's real `__afl_area_ptr` into the harness global.
- Restore the baseline bitmap before prerequisite calls.
- Save the baseline after prerequisites and before the target call.
- Guard all bitmap operations for NULL so standalone verification works without Frida.

Do not include coverage reset code for single-function harnesses without prerequisites.

## Standalone Verification

Run the built harness on device before Phase 4. Required checks:

1. Device is online and writable.
2. `build/fuzz`, libraries, APK, and seed exist.
3. `LD_LIBRARY_PATH` points to absolute `DEVICE_DIR/lib`.
4. Smoke input exits successfully or produces an explainable target exception.
5. Logs include stdout/stderr and relevant logcat output.

Return FAIL when linker errors, missing libraries, JNI signature mismatch, unresolved handles, or crashes prevent standalone execution.
