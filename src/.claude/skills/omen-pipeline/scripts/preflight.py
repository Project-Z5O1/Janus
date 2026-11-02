#!/usr/bin/env python3
"""Read-only environment and artifact preflight for Omen pipeline stages."""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any

STAGES = ("parser", "argument-construction", "harness", "fuzzing")


class Preflight:
    def __init__(self, root: Path, args: argparse.Namespace):
        self.root = root
        self.args = args
        self.checks: list[dict[str, str]] = []

    def add(self, name: str, ok: bool, detail: str, warning: bool = False) -> None:
        status = "PASS" if ok else ("WARN" if warning else "FAIL")
        self.checks.append({"name": name, "status": status, "detail": detail})

    def command(self, name: str, required: bool = True) -> None:
        path = shutil.which(name)
        self.add(f"command:{name}", bool(path), path or "not found in PATH", warning=not required)

    def file(self, name: str, relative: str, executable: bool = False, warning: bool = False) -> None:
        path = self.root / relative
        ok = path.is_file() and (not executable or os.access(path, os.X_OK))
        suffix = " (not executable)" if path.is_file() and executable and not ok else ""
        self.add(name, ok, f"{relative}{suffix}" if ok else f"missing or invalid: {relative}", warning)

    def directory(self, name: str, relative: str, warning: bool = False) -> None:
        path = self.root / relative
        self.add(name, path.is_dir(), relative if path.is_dir() else f"missing: {relative}", warning)

    def common(self) -> None:
        self.command("python3")

    def parser(self) -> None:
        for command in ("java", "unzip", "adb"):
            self.command(command)
        self.file("tool:jniinfo", "mcptool/jniinfo.py")
        self.file("tool:jadx", "retool/jadx-1.5.4/bin/jadx", executable=True)
        self.file("tool:apktool", "retool/apktool_3.0.1.jar")
        self.file(
            "tool:ghidra",
            "retool/ghidra_12.0.3_PUBLIC/support/analyzeHeadless",
            executable=True,
        )
        self.file("tool:ghidra-export", "retool/ghidra-scripts/ExportSoDecomp.java")
        self.file("tool:parser", ".agents/skills/cross-language-parser/scripts/preprocess_apk.sh")
        self.file("tool:binding-builder", ".agents/skills/cross-language-parser/scripts/build_binding_table.py")
        self.file("tool:state", ".agents/skills/omen-pipeline/scripts/omen_state.py")

    def build_tools(self) -> None:
        self.command("cmake")
        self.command("adb")
        ndk = os.environ.get("ANDROID_NDK_HOME") or os.environ.get("ANDROID_NDK_ROOT")
        if not ndk:
            self.add("env:android-ndk", False, "set ANDROID_NDK_HOME or ANDROID_NDK_ROOT")
            return
        toolchain = Path(ndk).expanduser() / "build/cmake/android.toolchain.cmake"
        self.add("env:android-ndk", toolchain.is_file(), str(toolchain) if toolchain.is_file() else f"invalid NDK: {ndk}")

    def harness_assets(self) -> None:
        for name in ("fuzz.c", "afl.js", "afl-debug.js", "CMakeLists.txt", "build.sh", "build-debug.sh", "fuzzing.sh", "fuzzing-debug.sh", "include/jenv.h", "lib/libjenv.so"):
            self.file(f"asset:{name}", f"assets/{name}")

    def afl_assets(self) -> None:
        candidates = []
        if os.environ.get("OMEN_AFL_DIR"):
            candidates.append(Path(os.environ["OMEN_AFL_DIR"]).expanduser())
        candidates.append(self.root / "assets/afl-binaries")
        for binary in ("afl-fuzz", "afl-frida-trace.so"):
            found = [str(directory / binary) for directory in candidates if (directory / binary).is_file()]
            self.add(f"afl:{binary}", bool(found), found[0] if found else "set OMEN_AFL_DIR or install under assets/afl-binaries")
            if not found:
                continue
            path = found[0]
            if binary == "afl-fuzz":
                self.add(f"afl:{binary}:executable", os.access(path, os.X_OK), path)
            file_command = shutil.which("file")
            if file_command:
                inspected = subprocess.run(
                    [file_command, path], text=True, capture_output=True, timeout=10, check=False
                )
                description = inspected.stdout.strip()
                lowered = description.lower()
                arm64 = "elf" in lowered and ("aarch64" in lowered or "arm64" in lowered)
                self.add(f"afl:{binary}:arm64", inspected.returncode == 0 and arm64, description or "unable to inspect binary")
        showmap = any((directory / "afl-showmap").is_file() for directory in candidates)
        self.add("afl:afl-showmap", showmap, "available" if showmap else "optional; short afl-fuzz coverage is the alternative", warning=True)

    def package_artifacts(self, require_jni: bool) -> None:
        if not self.args.package:
            self.add("scope:package", False, "--package is required for artifact preflight")
            return
        package = self.args.package
        base = f"target_APK/{package}"
        self.file("artifact:base-apk", f"{base}/base.apk")
        self.file("artifact:jni-list", f"{base}/jni_list.json")
        self.file("artifact:summary", f"{base}/summary.json")
        self.directory("artifact:package-libs", f"{base}/lib/arm64-v8a")
        if require_jni:
            if not self.args.jni_name:
                self.add("scope:jni", False, "--jni-name is required for this stage")
                return
            work = f"{base}/{self.args.jni_name}"
            self.file("artifact:call-sequence", f"{work}/call_sequence.md")
            self.file("artifact:argument-plan", f"{work}/argument_plan.md")
            self.file("artifact:fuzz-source", f"{work}/fuzz.c")
            self.file("artifact:afl-js", f"{work}/afl.js")
            self.file("artifact:fuzz-binary", f"{work}/build/fuzz", executable=True)
            seed_dir = self.root / work / "seeds"
            seeds = [path for path in seed_dir.glob("*") if path.is_file() and path.stat().st_size > 0]
            self.add("artifact:seeds", bool(seeds), f"non-empty seeds: {len(seeds)}" if seeds else f"no non-empty seeds in {work}/seeds")
            self.file("artifact:libjenv", f"{work}/lib/libjenv.so")
            self.buffer_consistency(self.root / work)
        elif self.args.jni_name:
            self.file("artifact:call-sequence", f"{base}/{self.args.jni_name}/call_sequence.md")

    def buffer_consistency(self, work: Path) -> None:
        fuzz = work / "fuzz.c"
        afl = work / "afl.js"
        if not fuzz.is_file() or not afl.is_file():
            return
        fuzz_text = fuzz.read_text(encoding="utf-8", errors="replace")
        afl_text = afl.read_text(encoding="utf-8", errors="replace")
        fuzz_match = re.search(r"^\s*#define\s+BUFFER_SIZE\s+(\d+)", fuzz_text, re.MULTILINE)
        afl_match = re.search(r"^\s*#define\s+BUF_LEN\s+(\d+)", afl_text, re.MULTILINE)
        if not fuzz_match or not afl_match:
            self.add("artifact:buffer-size", False, "BUFFER_SIZE or BUF_LEN could not be extracted")
            return
        ok = fuzz_match.group(1) == afl_match.group(1)
        self.add(
            "artifact:buffer-size",
            ok,
            f"BUFFER_SIZE={fuzz_match.group(1)}, BUF_LEN={afl_match.group(1)}",
        )

    def device(self) -> None:
        adb = shutil.which("adb")
        if not adb:
            return
        try:
            result = subprocess.run(
                [adb, "devices"], text=True, capture_output=True, timeout=10, check=False
            )
        except (OSError, subprocess.TimeoutExpired) as exc:
            self.add("device:enumeration", False, str(exc))
            return
        serials = [
            line.split()[0]
            for line in result.stdout.splitlines()[1:]
            if line.strip().endswith("\tdevice")
        ]
        serial = self.args.device or os.environ.get("OMEN_DEVICE_ID")
        if not serial:
            if len(serials) == 1:
                serial = serials[0]
            elif not serials:
                self.add("device:online", False, "no online adb device")
                return
            else:
                self.add("device:serial", False, "multiple devices; set --device or OMEN_DEVICE_ID")
                return
        if serial not in serials:
            self.add("device:online", False, f"device not online: {serial}")
            return
        self.add("device:online", True, serial)
        abi = subprocess.run(
            [adb, "-s", serial, "shell", "getprop", "ro.product.cpu.abi"],
            text=True,
            capture_output=True,
            timeout=10,
            check=False,
        )
        value = abi.stdout.strip()
        self.add("device:arm64", abi.returncode == 0 and "arm64" in value, value or "unable to read ABI")
        identity = subprocess.run(
            [adb, "-s", serial, "shell", "id"],
            text=True,
            capture_output=True,
            timeout=10,
            check=False,
        )
        self.add("device:shell", identity.returncode == 0, identity.stdout.strip() or identity.stderr.strip())
        writable = subprocess.run(
            [adb, "-s", serial, "shell", "test -w /data/local/tmp"],
            text=True,
            capture_output=True,
            timeout=10,
            check=False,
        )
        self.add("device:writable", writable.returncode == 0, "/data/local/tmp")
        locked = subprocess.run(
            [adb, "-s", serial, "shell", "test -d /data/local/tmp/omen/.fuzzing.lock"],
            text=True,
            capture_output=True,
            timeout=10,
            check=False,
        )
        self.add("device:slot", locked.returncode != 0, "available" if locked.returncode != 0 else "Omen device lock exists")

    def run(self) -> int:
        self.common()
        stages = STAGES if self.args.stage == "all" else (self.args.stage,)
        if "parser" in stages:
            self.parser()
        if any(stage in stages for stage in ("argument-construction", "harness")):
            self.build_tools()
            self.harness_assets()
        if any(stage in stages for stage in ("harness", "fuzzing")):
            self.afl_assets()
        if self.args.stage in {"argument-construction", "harness", "fuzzing"} or (self.args.stage == "all" and self.args.package):
            self.package_artifacts(require_jni=self.args.stage in {"harness", "fuzzing"} or (self.args.stage == "all" and bool(self.args.jni_name)))
        self.device()

        failed = any(check["status"] == "FAIL" for check in self.checks)
        result = {
            "ok": not failed,
            "stage": self.args.stage,
            "repo_root": str(self.root),
            "checks": self.checks,
        }
        if self.args.json:
            print(json.dumps(result, ensure_ascii=False, indent=2))
        else:
            for check in self.checks:
                print(f"[{check['status']}] {check['name']}: {check['detail']}")
            print(f"Preflight {'passed' if not failed else 'failed'}: {self.args.stage}")
        return 1 if failed else 0


def discover_root(explicit: str | None) -> Path:
    if explicit or os.environ.get("OMEN_ROOT"):
        root = Path(explicit or os.environ["OMEN_ROOT"]).expanduser().resolve()
        if root.is_dir():
            return root
        raise FileNotFoundError(f"invalid repository root: {root}")
    starts = (Path.cwd().resolve(), Path(__file__).resolve().parent)
    for start in starts:
        for candidate in (start, *start.parents):
            if (candidate / "AGENTS.md").is_file() and (candidate / ".agents" / "skills").is_dir():
                return candidate
    raise FileNotFoundError("cannot locate Omen root; set OMEN_ROOT or use --repo-root")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root")
    parser.add_argument("--stage", choices=(*STAGES, "all"), required=True)
    parser.add_argument("--package")
    parser.add_argument("--jni-name")
    parser.add_argument("--device")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    if args.jni_name and not args.package:
        parser.error("--jni-name requires --package")
    root = discover_root(args.repo_root)
    return Preflight(root, args).run()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.TimeoutExpired) as exc:
        print(json.dumps({"ok": False, "error": str(exc)}, ensure_ascii=False), file=sys.stderr)
        raise SystemExit(1)
