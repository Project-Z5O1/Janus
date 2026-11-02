"""
JNI Info CLI Module

This module wraps the essential functionality of
`static_analysis/preprocess.py` for a single APK path and exposes it through a
CLI:

- `extract`: Extract JNI signatures and find JNI offsets on device
- `parse`: Parse an existing `signatures_libraries_offsets.txt`

Prerequisites (same as preprocess.py/JNIOffset pipeline):
1) Local: `jadx`, `java`, `unzip` available in PATH (for signature extraction)
2) Device: rooted Android device; Termux installed with clang++ at
   `/data/data/com.termux/files/usr/bin/clang++`
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import re
import shutil
import subprocess
import sys
import uuid
from dataclasses import dataclass
from typing import Any, Optional

# NOTE: Do not import the repo-root `adb.py` here: it wraps functions as FastMCP tools.
# We need callable helpers, so we reuse the original (non-MCP) adb helper module from
# `static_analysis/adb.py`.
from static_analysis import adb as adb


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    stream=sys.stderr,
)
log = logging.getLogger(__name__)


MCP_TOOL_DIR = os.path.dirname(__file__)
OMEN_ROOT = os.path.abspath(os.environ.get("OMEN_ROOT") or os.path.join(MCP_TOOL_DIR, ".."))
STATIC_ANALYSIS_DIR = os.path.join(MCP_TOOL_DIR, "static_analysis")
TARGET_APK_DIR = os.path.join(OMEN_ROOT, "target_APK")

REMOTE_FOLDER = "/data/local/tmp/offsetFinding/"
TERMUX_CLANGXX = "/data/data/com.termux/files/usr/bin/clang++"

DEFAULT_AAPT_EXECUTABLE = (
    os.environ.get("AAPT") or os.environ.get("AAPT_PATH") or "aapt"
)


SPECIAL_LOCATIONS: dict[str, list[str]] = {
    "com.whatsapp": ["/data/data/com.whatsapp/files/decompressed/libs.spk.zst/"],
    "com.facebook.lite": ["/data/data/com.facebook.lite/lib-compressed/"],
    "com.facebook.orca": ["/data/data/com.facebook.orca/lib-compressed/"],
    "com.instagram.android": ["/data/data/com.instagram.android/lib-compressed/"],
    "com.facebook.pages.app": ["/data/data/com.facebook.pages.app/lib-compressed/"],
    "com.microsoft.office.officehubrow": [
        "/data/data/com.microsoft.office.officehubrow/files/data/applibs/"
    ],
    "com.whatsapp.w4b": [
        "/data/data/com.whatsapp.w4b/files/decompressed/libs.spk.zst/"
    ],
}


@dataclass(frozen=True)
class _CmdResult:
    returncode: int
    stdout: str
    stderr: str


def _resolve_path(path: str) -> str:
    return os.path.abspath(os.path.expanduser(os.path.expandvars(path)))


def _validate_apk_path(apk_path: str) -> tuple[bool, str]:
    resolved = _resolve_path((apk_path or "").strip())
    if not os.path.exists(resolved):
        return False, f"Error: APK file does not exist - {resolved}"
    if not os.path.isfile(resolved):
        return False, f"Error: Path is not a file - {resolved}"
    return True, resolved


def _run_local(
    cmd: list[str],
    timeout: Optional[int] = None,
    cwd: Optional[str] = None,
    env: Optional[dict] = None,
) -> _CmdResult:
    completed = subprocess.run(
        cmd, capture_output=True, text=True, timeout=timeout, cwd=cwd, env=env
    )
    return _CmdResult(
        returncode=completed.returncode,
        stdout=(completed.stdout or "").strip("\n"),
        stderr=(completed.stderr or "").strip("\n"),
    )


def _infer_package_name_from_aapt(apk_path: str) -> Optional[str]:
    aapt_exec = DEFAULT_AAPT_EXECUTABLE
    if not shutil.which(aapt_exec):
        return None
    r = _run_local([aapt_exec, "dump", "badging", apk_path], timeout=30)
    if r.returncode != 0:
        return None
    m = re.search(r"package: name='([^']+)'", r.stdout)
    return m.group(1) if m else None


def _infer_package_name_from_androguard(apk_path: str) -> Optional[str]:
    try:
        from androguard.core.apk import APK  # type: ignore
    except ModuleNotFoundError:
        return None
    try:
        apk = APK(apk_path)
        pkg = apk.get_package()
        return pkg if pkg else None
    except Exception:
        return None


def _infer_package_name(apk_path: str) -> Optional[str]:
    pkg = _infer_package_name_from_aapt(apk_path)
    if pkg:
        return pkg
    return _infer_package_name_from_androguard(apk_path)


def _ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def _safe_unlink(path: str) -> None:
    try:
        os.unlink(path)
    except FileNotFoundError:
        return


def _safe_rmtree(path: str) -> None:
    shutil.rmtree(path, ignore_errors=True)


def _sanitize_remote_component(value: str, max_len: int = 48) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9._-]+", "_", (value or "").strip())
    cleaned = re.sub(r"_+", "_", cleaned).strip("._-")
    if not cleaned:
        cleaned = "unnamed"
    return cleaned[:max_len]


def _build_remote_workspace_root(app_id: str) -> str:
    token = uuid.uuid4().hex[:12]
    app_part = _sanitize_remote_component(app_id, max_len=48)
    return f"{REMOTE_FOLDER.rstrip('/')}/{app_part}_{token}/"


def _prepare_app_workspace(
    app_id: str, apk_path: str, clean: bool, output_dir: Optional[str] = None
) -> str:
    """
    Ensure `<output_dir>/<app_id>/base.apk` exists and return the workspace directory.
    If clean=True, remove known generated artifacts to make the pipeline idempotent.

    Args:
        app_id: Package name / app identifier
        apk_path: Path to the APK file
        clean: Whether to clean previous artifacts
        output_dir: Custom output directory. Defaults to TARGET_APK_DIR if not specified.
    """
    base_dir = _resolve_path(output_dir) if output_dir else TARGET_APK_DIR
    _ensure_dir(base_dir)
    app_dir = os.path.join(base_dir, app_id)
    _ensure_dir(app_dir)

    if clean:
        _safe_rmtree(os.path.join(app_dir, "base"))
        _safe_rmtree(os.path.join(app_dir, "base_apk"))
        _safe_rmtree(os.path.join(app_dir, "lib"))
        _safe_unlink(os.path.join(app_dir, "base.zip"))
        _safe_unlink(os.path.join(app_dir, "signatures_pattern.txt"))
        _safe_unlink(os.path.join(app_dir, "signatures_libraries_offsets.txt"))

    shutil.copy2(apk_path, os.path.join(app_dir, "base.apk"))
    return app_dir


def _get_mangled_signature(sig: str) -> str:
    # Mirrors the lightweight mapping used by static_analysis/preprocess.py
    try:
        args = sig.split(":", 1)[1]
    except Exception:
        return ""
    args_list = args.split(",")
    type_mapping = {
        "boolean": "Z",
        "byte": "B",
        "char": "C",
        "short": "S",
        "int": "I",
        "long": "J",
        "float": "F",
        "double": "D",
        "String": "Ljava_lang_String_2",
        "rest": "Ljava_lang_Object_2",
    }
    mangled = ""
    for arg in args_list:
        if not arg:
            continue
        if arg in type_mapping:
            mangled += type_mapping[arg]
        elif arg.endswith("[]"):
            base = arg[:-2]
            if base in type_mapping:
                mangled += "_3" + type_mapping[base]
            else:
                mangled += "_3" + type_mapping["rest"]
        else:
            mangled += type_mapping["rest"]
    return mangled


def _fix_overloaded_signatures(signatures_path: str) -> None:
    if not os.path.exists(signatures_path):
        return
    with open(signatures_path, "r", encoding="utf-8", errors="replace") as f:
        lines = [ln.strip("\n") for ln in f.read().splitlines() if ln.strip()]

    # Find duplicates by JNI function name (before the space)
    by_name: dict[str, list[int]] = {}
    for idx, line in enumerate(lines):
        parts = line.split(" ")
        if len(parts) < 2:
            continue
        by_name.setdefault(parts[0], []).append(idx)

    updated = list(lines)
    for fname, idxs in by_name.items():
        if len(idxs) <= 1:
            continue
        for idx in idxs:
            parts = lines[idx].split(" ")
            if len(parts) < 2:
                continue
            sig = parts[1]
            new_name = f"{fname}__{_get_mangled_signature(sig)}"
            updated[idx] = f"{new_name} {sig}"

    with open(signatures_path, "w", encoding="utf-8") as f:
        f.write("\n".join(updated) + ("\n" if updated else ""))


def _check_local_prereqs_for_signatures() -> Optional[str]:
    missing = []
    for exe in ("bash", "unzip", "java", "jadx"):
        if not shutil.which(exe):
            missing.append(exe)
    if missing:
        return (
            f"Missing local executables for signature extraction: {', '.join(missing)}"
        )
    script = os.path.join(
        STATIC_ANALYSIS_DIR, "NativeSignatures", "analyze_native_signatures.sh"
    )
    if not os.path.exists(script):
        return f"Missing script: {script}"
    return None


def _extract_signatures_with_native_signatures(
    app_id: str, timeout_sec: int = 1800, output_dir: Optional[str] = None
) -> tuple[bool, str]:
    err = _check_local_prereqs_for_signatures()
    if err:
        return False, err

    base_dir = _resolve_path(output_dir) if output_dir else TARGET_APK_DIR
    script = os.path.join(
        STATIC_ANALYSIS_DIR, "NativeSignatures", "analyze_native_signatures.sh"
    )
    # Pass custom output directory to the script via environment variable
    env = os.environ.copy()
    env["TARGET_APK_DIR"] = base_dir
    r = _run_local(["bash", script, app_id], timeout=timeout_sec, cwd=None, env=env)
    details = (r.stdout + ("\n" + r.stderr if r.stderr else "")).strip()
    if r.returncode != 0:
        return (
            False,
            f"NativeSignatures failed (code={r.returncode}).\n{details}".strip(),
        )

    sig_path = os.path.join(base_dir, app_id, "signatures_pattern.txt")
    if not os.path.exists(sig_path):
        return (
            False,
            f"signatures_pattern.txt not generated at: {sig_path}\n{details}".strip(),
        )

    _fix_overloaded_signatures(sig_path)
    return True, details or "OK"


def _ensure_device_id(device_id: Optional[str]) -> tuple[bool, str]:
    if device_id:
        return True, device_id
    ids = adb.get_device_ids()
    if not ids:
        return False, "Error: No connected adb devices found (device_id is required)."
    if len(ids) > 1:
        return False, f"Error: Multiple devices found {ids}; please specify device_id."
    # static_analysis/adb.py returns bytes ids
    first = ids[0]
    if isinstance(first, bytes):
        return True, first.decode("utf-8", errors="replace")
    return True, str(first)


def _device_has_root(device_id: str) -> bool:
    try:
        out, _ = adb.execute_privileged_command("id", device_id=device_id, timeout=5)
    except Exception:
        return False
    return b"root" in (out or b"")


def _remote_has_jnioffset(device_id: str, remote_folder: str = REMOTE_FOLDER) -> bool:
    # Ensure both the binary and the launcher script exist.
    try:
        out, _ = adb.execute_privileged_command(
            f"test -x {remote_folder}jniOffsetFinder && test -f {remote_folder}findJNIOffsets.sh && echo OK",
            device_id=device_id,
            timeout=10,
        )
    except Exception:
        return False
    return b"OK" in (out or b"")


def _init_jnioffset(
    device_id: str, timeout_sec: int = 300, remote_folder: str = REMOTE_FOLDER
) -> tuple[bool, str]:
    if not _device_has_root(device_id):
        return (
            False,
            f"Error: device {device_id} has no root access (required for JNIOffset).",
        )

    # Ensure Termux clang++ exists
    try:
        out, _ = adb.execute_privileged_command(
            f"test -x {TERMUX_CLANGXX} && echo OK", device_id=device_id, timeout=10
        )
        if b"OK" not in out:
            return False, f"Error: missing Termux clang++ on device at {TERMUX_CLANGXX}"
    except Exception as e:
        return False, f"Error: failed to verify clang++ on device: {e}"

    local_dir = os.path.join(STATIC_ANALYSIS_DIR, "JNIOffset")
    harness = os.path.join(local_dir, "harness")
    example_harness = os.path.join(local_dir, "example_harness")
    cpp = os.path.join(local_dir, "cpp", "jniOffsetFinder.cpp")
    hdr = os.path.join(local_dir, "cpp", "jniOffsetFinder.h")
    script = os.path.join(local_dir, "findJNIOffsets.sh")

    for p in (harness, example_harness, cpp, hdr, script):
        if not os.path.exists(p):
            return False, f"Error: missing local JNIOffset file: {p}"

    try:
        adb.execute_privileged_command(
            f"rm -rf {remote_folder} && mkdir -p {remote_folder}",
            device_id=device_id,
            timeout=30,
        )
    except Exception as e:
        return False, f"Error: failed to create remote folder {remote_folder}: {e}"

    # Push tool files
    push_logs = []
    for p in (harness, example_harness, cpp, hdr):
        out, err = adb.push_privileged(p, remote_folder, device_id=device_id)
        out_s = (out or b"").decode("utf-8", errors="replace").strip()
        err_s = (err or b"").decode("utf-8", errors="replace").strip()
        msg = out_s
        if err_s:
            msg = f"{msg}\n[stderr]: {err_s}".strip() if msg else f"[stderr]: {err_s}"
        if msg:
            push_logs.append(msg)

    # Compile on device (Termux clang++)
    compile_cmd = (
        f"{TERMUX_CLANGXX} -std=c++17 -v -Wl,--export-dynamic -g -O0 "
        f"{remote_folder}jniOffsetFinder.cpp -o {remote_folder}jniOffsetFinder"
    )
    try:
        out, err = adb.execute_privileged_command(
            compile_cmd, device_id=device_id, timeout=timeout_sec
        )
        compile_out = (out or b"").decode("utf-8", errors="replace").strip()
        compile_err = (err or b"").decode("utf-8", errors="replace").strip()
        if compile_err:
            compile_out = f"{compile_out}\n[stderr]: {compile_err}".strip()
    except Exception as e:
        return False, f"Error: failed compiling jniOffsetFinder on device: {e}"

    # Push and chmod launcher script
    out, err = adb.push_privileged(script, remote_folder, device_id=device_id)
    out_s = (out or b"").decode("utf-8", errors="replace").strip()
    err_s = (err or b"").decode("utf-8", errors="replace").strip()
    msg = out_s
    if err_s:
        msg = f"{msg}\n[stderr]: {err_s}".strip() if msg else f"[stderr]: {err_s}"
    if msg:
        push_logs.append(msg)
    try:
        adb.execute_privileged_command(
            f"chmod +x {remote_folder}findJNIOffsets.sh",
            device_id=device_id,
            timeout=10,
        )
    except Exception as e:
        return False, f"Error: failed to chmod findJNIOffsets.sh: {e}"

    logs = "\n".join([l for l in push_logs if l]).strip()
    if compile_out:
        logs = f"{logs}\n{compile_out}".strip() if logs else compile_out
    return True, logs or "OK"


def _extract_offsets_on_device(
    app_id: str,
    device_id: str,
    remote_folder: str,
    cleanup_remote: bool = True,
    library_specific: Optional[str] = None,
    per_library_timeout_sec: int = 60,
    output_dir: Optional[str] = None,
) -> tuple[bool, str]:
    if not _device_has_root(device_id):
        return (
            False,
            f"Error: device {device_id} has no root access (required for JNIOffset).",
        )

    base_dir = _resolve_path(output_dir) if output_dir else TARGET_APK_DIR
    app_dir = os.path.join(base_dir, app_id)
    sig_path = os.path.join(app_dir, "signatures_pattern.txt")
    if not os.path.exists(sig_path):
        return (
            False,
            f"Error: missing signatures_pattern.txt at {sig_path} (run signatures extraction first).",
        )

    libs_dir = os.path.join(app_dir, "lib", "arm64-v8a")
    if not os.path.isdir(libs_dir) and app_id in SPECIAL_LOCATIONS:
        # Best-effort: some apps keep arm64 libs in non-standard on-device locations.
        _ensure_dir(libs_dir)
        for custom_loc in SPECIAL_LOCATIONS.get(app_id, []):
            try:
                out, _ = adb.execute_privileged_command(
                    f"ls {custom_loc}", device_id=device_id, timeout=30
                )
                listing = (out or b"").decode("utf-8", errors="replace").splitlines()
            except Exception:
                continue
            for entry in listing:
                entry = entry.strip()
                if not entry.endswith(".so"):
                    continue
                local_lib_path = os.path.join(libs_dir, entry)
                if os.path.exists(local_lib_path):
                    continue
                try:
                    adb.pull_privileged(
                        os.path.join(custom_loc, entry), libs_dir, device_id=device_id
                    )
                except Exception:
                    continue

    if not os.path.isdir(libs_dir):
        return (
            False,
            f"Error: missing native libraries at {libs_dir} (APK may not ship arm64 libs).",
        )

    libraries = [f for f in os.listdir(libs_dir) if f.endswith(".so")]
    if library_specific:
        libraries = [library_specific]
    if not libraries:
        return False, f"Error: no .so libraries found under {libs_dir}"

    # Ensure remote JNIOffset is present (auto-init if missing)
    if not _remote_has_jnioffset(device_id, remote_folder=remote_folder):
        ok, msg = _init_jnioffset(device_id, remote_folder=remote_folder)
        if not ok:
            return False, msg

    remote_app_dir = f"{remote_folder}{app_id}"
    try:
        adb.execute_privileged_command(
            f"rm -rf {remote_app_dir} && mkdir -p {remote_app_dir}",
            device_id=device_id,
            timeout=30,
        )
    except Exception as e:
        return False, f"Error: failed to create remote app dir {remote_app_dir}: {e}"

    # Upload required files
    push_logs = []
    for p, is_dir in (
        (os.path.join(app_dir, "base.apk"), False),
        (sig_path, False),
        (os.path.join(app_dir, "lib"), True),
    ):
        out, err = adb.push_privileged(
            p, remote_app_dir, is_directory=is_dir, device_id=device_id
        )
        out_s = (out or b"").decode("utf-8", errors="replace").strip()
        err_s = (err or b"").decode("utf-8", errors="replace").strip()
        msg = out_s
        if err_s:
            msg = f"{msg}\n[stderr]: {err_s}".strip() if msg else f"[stderr]: {err_s}"
        if msg:
            push_logs.append(msg)

    ld_preload = ""
    if "libc++_shared.so" in libraries:
        ld_preload = f"{remote_app_dir}/lib/arm64-v8a/libc++_shared.so"

    # Some APKs bundle copies of OS-core libs (e.g. an old liblog.so without
    # versioned LIBLOG_M symbols). In the flat LD_LIBRARY_PATH namespace used
    # here they shadow the system ones and can abort the loader when libart.so
    # resolves its versioned dependencies ("cannot locate symbol
    # __android_log_error_write"). On a real device these app copies never
    # satisfy ART/system dependencies, so drop them to match that behavior.
    # Offsets are library-relative and unaffected by dependency resolution.
    os_core_libs = [
        "liblog.so",
        "libart.so",
        "libandroid_runtime.so",
        "libc.so",
        "libm.so",
        "libdl.so",
        "libstdc++.so",
    ]
    # Check the actually-uploaded directory contents (the whole lib dir is
    # pushed even when extracting a single library), not the `libraries` list.
    uploaded = set(os.listdir(libs_dir)) if os.path.isdir(libs_dir) else set()
    shadowing = [
        f"{remote_app_dir}/lib/arm64-v8a/{name}"
        for name in os_core_libs
        if name in uploaded
    ]
    if shadowing:
        try:
            adb.execute_privileged_command(
                f"rm -f {' '.join(shadowing)}",
                device_id=device_id,
                timeout=30,
            )
            push_logs.append(f"[shadowing] removed OS-core libs from app dir: {', '.join(shadowing)}")
        except Exception as e:  # removal failure is not fatal
            push_logs.append(f"[shadowing] failed to remove OS-core libs: {e}")

    # Run per-library extraction
    run_logs: list[str] = []
    for lib in libraries:
        cmd = f"{remote_folder}findJNIOffsets.sh {app_id} {lib} {remote_folder} {ld_preload}"
        try:
            out, err = adb.execute_privileged_command(
                cmd, device_id=device_id, timeout=per_library_timeout_sec
            )
            out_s = (out or b"").decode("utf-8", errors="replace").strip()
            err_s = (err or b"").decode("utf-8", errors="replace").strip()
            if err_s:
                out_s = f"{out_s}\n[stderr]: {err_s}".strip()
            if out_s:
                run_logs.append(f"[{lib}]\n{out_s}")
        except adb.DeviceTimeoutException:
            run_logs.append(
                f"[{lib}] timeout after {per_library_timeout_sec}s (continuing)"
            )
        except Exception as e:
            run_logs.append(f"[{lib}] error: {e}")

    # Pull results back
    local_out_path = os.path.join(app_dir, "signatures_libraries_offsets.txt")
    _safe_unlink(local_out_path)
    pull_out = adb.pull_privileged(
        f"{remote_app_dir}/signatures_libraries_offsets.txt",
        app_dir,
        device_id=device_id,
    )
    pull_res = (pull_out or b"").decode("utf-8", errors="replace").strip()

    # Cleanup remote dir
    if cleanup_remote:
        try:
            adb.execute_privileged_command(
                f"rm -rf {remote_folder}", device_id=device_id, timeout=30
            )
        except Exception:
            pass

    if not os.path.exists(local_out_path):
        # Create an empty file for consistent downstream behavior
        with open(local_out_path, "w", encoding="utf-8") as f:
            f.write("")
    else:
        _normalize_offsets_file(local_out_path)

    logs = "\n".join([l for l in push_logs if l]).strip()
    if pull_res:
        logs = f"{logs}\n{pull_res}".strip() if logs else pull_res
    if run_logs:
        logs = f"{logs}\n" + "\n".join(run_logs) if logs else "\n".join(run_logs)
    return True, logs or "OK"


def _parse_signature(signature: str) -> tuple[Optional[str], list[str]]:
    signature = (signature or "").strip()
    if ":" not in signature:
        return None, []
    ret, args = signature.split(":", 1)
    arg_types = [a for a in args.split(",") if a]
    return ret or None, arg_types


def _parse_jni_name(
    jni_name: str,
) -> tuple[Optional[str], Optional[str], Optional[str]]:
    """
    Best-effort parsing for names produced by NativeSignatures extractor:
    - Java_<pkg>_<Class>_<method>
    - underscores in original names are escaped as _1
    - inner class '$' may appear (or as _00024)
    - overloaded functions may contain '__<mangledSig>'
    """
    if not jni_name or not jni_name.startswith("Java_"):
        return None, None, None

    core = jni_name[len("Java_") :]
    # A literal "__" is either the overload boundary (separator + mangled
    # signature starting with a descriptor char, or empty suffix) or a
    # separator followed by an escape digit (_1/_2/_3), in which case the
    # underscore belongs to the method name itself (e.g. Class__1stop is
    # class Class, method _stop) and must not be treated as an overload.
    overload = None
    pos = core.find("__")
    while pos != -1:
        tail = core[pos + 2 :]
        if tail == "" or tail[0] in "ZBCSIJFDVL" or tail.startswith("_3"):
            core, overload = core[:pos], tail
            break
        pos = core.find("__", pos + 1)

    core = core.replace("_00024", "$")
    # Escape-aware tokenization: _1 -> '_', _2 -> ';', _3 -> '['; any other
    # '_' is a component separator. Java identifiers never start with a
    # digit, so an escape is never ambiguous with a separator.
    tokens: list[str] = []
    current: list[str] = []
    i = 0
    while i < len(core):
        ch = core[i]
        if ch == "_" and i + 1 < len(core) and core[i + 1] in "123":
            current.append({"1": "_", "2": ";", "3": "["}[core[i + 1]])
            i += 2
        elif ch == "_":
            tokens.append("".join(current))
            current = []
            i += 1
        else:
            current.append(ch)
            i += 1
    tokens.append("".join(current))
    if len(tokens) < 2:
        return None, None, overload
    method = tokens[-1]
    cls = ".".join(tokens[:-1])
    return cls or None, method or None, overload


def _parse_signatures_libraries_offsets(text: str) -> list[dict[str, Any]]:
    entries: list[dict[str, Any]] = []
    for line in (text or "").splitlines():
        line = line.strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) < 4:
            entries.append({"raw": line, "error": "unrecognized format"})
            continue
        jni_name, signature, library, offset_s = parts[0], parts[1], parts[2], parts[3]
        java_class, java_method, overload = _parse_jni_name(jni_name)
        ret_type, param_types = _parse_signature(signature)
        try:
            offset = int(offset_s, 0)
        except Exception:
            offset = None
        entries.append(
            {
                "jni_name": jni_name,
                "java_class": java_class,
                "java_method": java_method,
                "overload_mangled": overload,
                "return_type": ret_type,
                "param_types": param_types,
                "library": library,
                "offset": offset,
                "raw": line,
            }
        )
    return entries


def _normalize_offsets_file(path: str) -> None:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            raw = f.read()
    except Exception:
        return

    if not raw:
        return

    lines: list[str] = []
    for line in raw.splitlines():
        stripped = line.strip()
        if not stripped:
            lines.append(line)
            continue
        parts = stripped.split()
        if len(parts) < 4:
            lines.append(line)
            continue
        offset_s = parts[3]
        try:
            offset = int(offset_s, 0)
            parts[3] = f"0x{offset:X}"
            lines.append(" ".join(parts))
        except Exception:
            lines.append(line)

    try:
        with open(path, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
    except Exception:
        return


def preprocess_apk_jni_offsets(
    apk_path: str,
    package_name: str,
    device_id: str,
    output_dir: Optional[str] = None,
) -> dict[str, Any]:
    """
    One-shot pipeline: given `apk_path` + `package_name` + `device_id`,
    extract signatures and JNI offsets and return:
    - `raw`: the full `signatures_libraries_offsets.txt` text
    - `entries`: parsed entries (java class/method/params + so + offset)
    - `output_path`: the path to the generated output file

    Args:
        apk_path: Path to the APK file to analyze
        package_name: Android package name (e.g., com.example.app)
        device_id: ADB device ID for the rooted Android device
        output_dir: Optional custom output directory. If not specified, defaults to
                    `<script_dir>/target_APK/`. The output file will be at
                    `<output_dir>/<package_name>/signatures_libraries_offsets.txt`

    Behavior:
    - Always runs in init mode (re)uploading/compiling JNIOffset on device
    - Uses a per-run remote workspace root under `/data/local/tmp/offsetFinding/`
    - Always cleans up the per-run remote temp folder before returning
    """
    ok, resolved_apk = _validate_apk_path(apk_path)
    if not ok:
        return {"ok": False, "error": resolved_apk}

    app_id = (package_name or "").strip()
    if not app_id:
        return {"ok": False, "error": "Error: package_name is empty"}

    device_id = (device_id or "").strip()
    if not device_id:
        return {"ok": False, "error": "Error: device_id is empty"}

    # Resolve output directory
    resolved_output_dir = (
        _resolve_path(output_dir.strip()) if output_dir and output_dir.strip() else None
    )

    # Always reset workspace for correctness (APK path may change across calls).
    app_dir = _prepare_app_workspace(
        app_id, resolved_apk, clean=True, output_dir=resolved_output_dir
    )
    out_path = os.path.join(app_dir, "signatures_libraries_offsets.txt")

    sig_ok, sig_log = _extract_signatures_with_native_signatures(
        app_id, output_dir=resolved_output_dir
    )
    if not sig_ok:
        return {"ok": False, "error": sig_log}

    remote_folder = _build_remote_workspace_root(app_id)

    try:
        init_ok, init_log = _init_jnioffset(device_id, remote_folder=remote_folder)
        if not init_ok:
            return {"ok": False, "error": init_log}

        off_ok, off_log = _extract_offsets_on_device(
            app_id=app_id,
            device_id=device_id,
            remote_folder=remote_folder,
            cleanup_remote=True,
            library_specific=None,
            output_dir=resolved_output_dir,
        )
        if not off_ok:
            return {"ok": False, "error": off_log}

        with open(out_path, "r", encoding="utf-8", errors="replace") as f:
            raw = f.read().strip("\n")

        return {
            "ok": True,
            "raw": raw,
            "entries": _parse_signatures_libraries_offsets(raw),
            "output_path": out_path,
            "log": off_log,
        }
    finally:
        try:
            adb.execute_privileged_command(
                f"rm -rf {remote_folder}", device_id=device_id, timeout=30
            )
        except Exception:
            pass


def parse_signatures_libraries_offsets_file(path: str) -> dict[str, Any]:
    """
    Parse an existing signatures_libraries_offsets.txt file and return structured entries.
    """
    resolved = _resolve_path((path or "").strip())
    if not os.path.exists(resolved):
        return {"ok": False, "error": f"Error: file does not exist - {resolved}"}
    if not os.path.isfile(resolved):
        return {"ok": False, "error": f"Error: path is not a file - {resolved}"}
    with open(resolved, "r", encoding="utf-8", errors="replace") as f:
        raw = f.read().strip("\n")
    return {
        "ok": True,
        "path": resolved,
        "raw": raw,
        "entries": _parse_signatures_libraries_offsets(raw),
    }


def _build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="JNI info CLI: extract JNI offsets from APKs or parse existing result files."
    )
    subparsers = parser.add_subparsers(dest="command")

    extract_parser = subparsers.add_parser(
        "extract",
        help="Run JNI preprocessing pipeline for one APK and return JSON result.",
    )
    extract_parser.add_argument("--apk", required=True, help="Path to APK file")
    extract_parser.add_argument(
        "--package", required=True, help="Package name, e.g. com.example.app"
    )
    extract_parser.add_argument("--device", required=True, help="ADB device ID")
    extract_parser.add_argument(
        "--output-dir",
        required=False,
        help="Optional output directory; defaults to <repo>/target_APK",
    )

    parse_parser = subparsers.add_parser(
        "parse",
        help="Parse an existing signatures_libraries_offsets.txt and return JSON result.",
    )
    parse_parser.add_argument(
        "--path",
        required=True,
        help="Path to signatures_libraries_offsets.txt",
    )
    return parser


def main(argv: Optional[list[str]] = None) -> int:
    parser = _build_arg_parser()
    args = parser.parse_args(argv)

    if args.command == "extract":
        result = preprocess_apk_jni_offsets(
            apk_path=args.apk,
            package_name=args.package,
            device_id=args.device,
            output_dir=args.output_dir,
        )
    elif args.command == "parse":
        result = parse_signatures_libraries_offsets_file(path=args.path)
    else:
        parser.print_help(sys.stderr)
        return 1

    json.dump(result, sys.stdout, indent=2, ensure_ascii=False)
    sys.stdout.write("\n")
    return 0 if result.get("ok") else 1


if __name__ == "__main__":
    raise SystemExit(main())
