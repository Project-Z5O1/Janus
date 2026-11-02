# Omen

**Agentic JNI-aware fuzzing harness generation for Android native libraries.**

Omen is the engineering implementation of **Janus: Agentic Cross-Language Fuzzing Harness
Generation for Android Native Libraries**. Given an Android APK, Omen recovers Java–native
relationships, synthesizes the invocation context and argument objects required by each JNI
method, validates the resulting harness with execution feedback, and prepares it for AFL++
Frida Mode fuzzing on Android.

Omen combines **agent skills with executable helpers**. The skills govern evidence-based
reasoning, explicit intermediate artifacts, and external validation; deterministic helpers
support parsing, state management, Memory storage, builds, deployment, and metric collection.

## Why JNI-aware harnesses?

A native library shipped in an Android app inherits the app's process and permissions, but not
ART's managed-memory safety. Fuzzing its JNI entry points requires more than passing random
bytes to an exported function:

- **Hidden invocation context:** a target may require earlier Java or JNI calls, native
  initialization, field writes, or an opaque handle produced by a factory.
- **Complex argument semantics:** nested Java objects must be constructed through JNI with the
  state that the native implementation actually reads.
- **Shallow false success:** a harness can compile and exit normally without reaching the
  target's core logic.

Omen addresses these problems with two core synthesis algorithms and a feedback-driven
validation loop.

## Architecture

Omen implements Janus's three-stage workflow, supported by orchestration for target selection,
prioritization, state management, retries, and device scheduling.

```text
APK
  │
  ▼
Phase 1 — Cross-Language Parser
  Java + Smali + native pseudocode + JNI Binding Table
  │
  ▼
Phase 2 — Harness Synthesizer
  Structure-Aware Retriever
  ├─ Algorithm 1: Call Sequence Extraction
  ├─ Algorithm 2: Recursive Argument Object Construction
  └─ Harness Memory: verified components reused across JNI methods
  │
  ▼
Phase 3 — Feedback-Driven Harness Refiner
  fixed template → build → semantic review → runtime → coverage
  failures return to the component responsible for the defect
  │
  ▼
AFL++ Frida Mode fuzzing on Android
```

### The two core algorithms

**Algorithm 1 — Call Sequence Extraction: how should the target be invoked?**  
The Dependency-Driven Call Sequence Extractor starts with unsatisfied explicit and implicit
dependencies, retrieves evidence for provider calls, and iteratively builds an annotated
prerequisite sequence. Each call carries `role`, `source`, and `binding` annotations; the
complete sequence and annotations pass local resolution and global review. Native pointers and
opaque handles come from verified factories rather than fuzzer-generated integers or `NULL`
placeholders.

**Algorithm 2 — Recursive Argument Object Construction: how should arguments be built?**  
Resolve leaves by emitting a value already bound by Algorithm 1's annotations, directly
synthesizing an unbound leaf through JNI, or reusing a compatible verified Memory entry. For
composite objects, rank real constructor/factory paths by app usage and native-read field
coverage, recursively construct child arguments, backtrack when a candidate fails, and assemble
objects bottom-up. Omen publishes recipes to Harness Memory after symbol checks, an independent
build, and an on-device smoke test.

## Repository guide

| Path | Contents |
|---|---|
| [`AGENTS.md`](AGENTS.md) | Architecture, invariants, stage contracts, and operating rules |
| [`.agents/skills/`](.agents/skills/) | Eight canonical skills; start with each `SKILL.md` |
| [`.agents/commands/`](.agents/commands/) | Pipeline command launcher |
| [`.opencode/`](.opencode/) and [`.claude/`](.claude/) | Client adapters for the canonical skills |
| [`assets/`](assets/) | Harness templates, build/fuzzing scripts, JNI runtime assets, and AFL device binaries |
| [`retool/`](retool/) | JADX, apktool, Ghidra, export scripts, and Smali language-server support |
| [`mcptool/`](mcptool/) | JNI signature/offset extraction and supporting analysis utilities |
| [`apks/`](apks/) | Default APK input location |
| [`target_APK/`](target_APK/) | Per-package analysis, harnesses, Memory, state, and fuzzing artifacts |
| [`tests/`](tests/) | Unit tests and skill-contract validation |

## Prerequisites

### Host

- A Unix-like environment with Bash, Python 3.10+, and standard Unix utilities.
- Java 21 for the bundled Ghidra and JADX toolchain.
- Android SDK Platform Tools with `adb` on `PATH`; `unzip` and `java` must also be available.
- `jadx` on `PATH` for signature extraction.
- CMake and an Android NDK for harness builds.
- Optional semantic-retrieval tools: `clangd`, `jdtls`, `ast-grep`, and Node.js.

Run from the repository root. The provided harness build scripts target **Android API 31** and
**`arm64-v8a`**.

### Android device

- A rooted ARM64 Android device or emulator selected uniquely through `adb`.
- Termux `clang++` at `/data/data/com.termux/files/usr/bin/clang++` for JNI binding extraction.
- A writable `/data/local/tmp` with permissions for harness execution and Frida-based AFL++.
- Compatible ARM64 AFL++/Frida device binaries. `assets/afl-binaries/` provides a set of device
  binaries; verify their compatibility with the selected device.

Run at most one Omen fuzzing task per device. Full parsing includes device-assisted JNI binding
extraction.

### Environment variables

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk       # or ANDROID_NDK_ROOT
export OMEN_DEVICE_ID=<adb-serial>                 # recommended when multiple devices exist

# Optional overrides:
export OMEN_ROOT=/path/to/Omen
export OMEN_AFL_DIR=/path/to/device-afl-binaries   # defaults to assets/afl-binaries
export OMEN_FUZZ_TIMEOUT=240                       # AFL run duration in seconds
```

`OMEN_PROFILE=release|debug` and `OMEN_AFL_EXTRA_ENV` provide additional execution controls for
the generated scripts. See the relevant stage skill before changing execution settings.

## Quick start

### 1. Check the environment

```bash
adb devices
python3 .agents/skills/omen-pipeline/scripts/preflight.py \
  --stage parser --device "$OMEN_DEVICE_ID"
```

Preflight checks the immediate environment for the selected stage. Before a full run, also
verify root access, Termux `clang++`, `jadx`, the NDK, and AFL/Frida compatibility.

### 2. Launch the pipeline

Open the repository in OpenCode, the paper's reference client, or another compatible agent
client, then run:

```text
/omen-pipeline /absolute/path/to/app.apk
```

To process `apks/*.apk` serially instead:

```text
/omen-pipeline
```

The orchestrator completes one package before moving to the next and coordinates per-JNI
synthesis, refinement, and fuzzing. Individual `SKILL.md` files define the inputs and acceptance
gates for single-stage work.

### 3. Inspect progress

```bash
python3 .agents/skills/omen-pipeline/scripts/omen_state.py \
  status --package <package>
```

`summary.json` schema v2 stores runtime state. Use `omen_state.py` for state transitions and
`memory_store.py` for Memory index updates. The status report provides per-stage scheduling
counts, aggregate outcome counts, and the next candidate.

### Parser-only entry point

Use the canonical parser wrapper for a standalone parsing stage:

```bash
bash .agents/skills/cross-language-parser/scripts/preprocess_apk.sh \
  --apk /absolute/path/to/app.apk \
  --package <package> \
  --device "$OMEN_DEVICE_ID" \
  --ghidra-jobs 4
```

The wrapper creates and validates `target_APK/<package>/` and initializes its runtime state.
Omen then applies target-selection and priority gates before call-sequence extraction, argument
construction, and harness refinement.

## Generated artifacts

```text
target_APK/<package>/
├── base.apk
├── jni_list.json                         # JNI Binding Table
├── signatures_libraries_offsets.txt      # binding evidence
├── src/{java,smali,native}/               # cross-language analysis inputs
├── targets.md / filtered.md / priority.md # selection and prioritization artifacts
├── summary.json                          # runtime state, schema v2
├── memory/                               # verified reusable components
└── <jni>/
    ├── call_sequence.md                  # Algorithm 1
    ├── argument_plan.md                  # Algorithm 2
    ├── object_recipes/                   # independently validated constructors
    ├── fuzz.c / afl.js / build/          # target-specific harness
    ├── harness_review.md                 # semantic and validation evidence
    ├── results/                          # fuzzer stats, crashes, and hangs
    └── fuzzing_metrics.json              # execution metrics
```

Markdown manifests and reviews supplement the machine-readable state with audit evidence.
Harnesses resolve target addresses using `dlopen`, the runtime library base, and the recorded
library-relative offset rather than symbol lookup.

## Tests and validation

Run the implementation checks from the repository root:

```bash
python3 tests/validate_skills.py
python3 -m unittest discover -s tests -p 'test_*.py' -v
```

These tests cover skill contracts, binding-table conversion, state transitions, and Memory
storage. After synthesis populates a target's harness and seed, use its build and fuzzing
wrappers under `target_APK/<package>/<jni>/` for target-specific device validation.
