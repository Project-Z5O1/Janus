#!/usr/bin/env python3
"""Crash-recoverable, serial state helper for Omen summary.json schema v2."""
from __future__ import annotations

import argparse
import contextlib
import fcntl
import json
import os
import shutil
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterator

MAX_RETRIES = 3
STAGE_ORDER = ("call-sequence", "argument-construction", "harness", "fuzzing")
SCHEDULING_ORDER = tuple(reversed(STAGE_ORDER))
STAGES: dict[str, dict[str, str]] = {
    "call-sequence": {
        "field": "call_sequence",
        "retries": "call_sequence_retries",
        "error": "call_sequence_last_error",
        "fix": "call_sequence_fix_applied",
    },
    "argument-construction": {
        "field": "argument_construction",
        "retries": "argument_construction_retries",
        "error": "argument_construction_last_error",
        "fix": "argument_construction_fix_applied",
    },
    "harness": {
        "field": "harness",
        "retries": "harness_retries",
        "error": "harness_last_error",
        "fix": "harness_fix_applied",
    },
    "fuzzing": {
        "field": "fuzzing",
        "retries": "fuzzing_retries",
        "error": "fuzzing_last_error",
        "fix": "fuzzing_fix_applied",
    },
}
ALIASES = {
    "call_sequence": "call-sequence",
    "callseq": "call-sequence",
    "phase2": "call-sequence",
    "argument_construction": "argument-construction",
    "arguments": "argument-construction",
    "phase3": "harness",
    "phase4": "fuzzing",
}


def emit(data: Any) -> None:
    print(json.dumps(data, ensure_ascii=False, indent=2))


def iso_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def normalize_stage(value: str) -> str:
    if value in STAGES:
        return value
    if value in ALIASES:
        return ALIASES[value]
    raise ValueError(f"unsupported stage: {value}")


def discover_repo_root(explicit: str | None = None) -> Path:
    if explicit:
        root = Path(explicit).expanduser().resolve()
        if not root.is_dir():
            raise FileNotFoundError(f"repository root does not exist: {root}")
        return root
    if os.environ.get("OMEN_ROOT"):
        root = Path(os.environ["OMEN_ROOT"]).expanduser().resolve()
        if root.is_dir():
            return root
        raise FileNotFoundError(f"OMEN_ROOT does not exist: {root}")

    candidates = [Path.cwd().resolve(), *Path.cwd().resolve().parents]
    script = Path(__file__).resolve()
    candidates.extend([script.parent, *script.parents])
    seen: set[Path] = set()
    for candidate in candidates:
        if candidate in seen:
            continue
        seen.add(candidate)
        if (candidate / ".agents" / "skills").is_dir() or (
            (candidate / "AGENTS.md").is_file() and (candidate / "target_APK").is_dir()
        ):
            return candidate
    raise FileNotFoundError("cannot locate Omen repository; set OMEN_ROOT or use --repo-root")


def safe_package(package: str) -> str:
    if (
        not package
        or package in {".", ".."}
        or "/" in package
        or "\\" in package
        or "\x00" in package
    ):
        raise ValueError(f"invalid package name: {package!r}")
    return package


def package_dir(root: Path, package: str) -> Path:
    return root / "target_APK" / safe_package(package)


def summary_path(root: Path, package: str) -> Path:
    return package_dir(root, package) / "summary.json"


def write_json_atomic(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temp_name = ""
    try:
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, prefix=f".{path.name}.", delete=False
        ) as handle:
            temp_name = handle.name
            json.dump(data, handle, ensure_ascii=False, indent=2)
            handle.write("\n")
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    finally:
        if temp_name and os.path.exists(temp_name):
            os.unlink(temp_name)


@contextlib.contextmanager
def locked_summary(root: Path, package: str, create: bool = False) -> Iterator[tuple[Path, dict[str, Any] | None]]:
    path = summary_path(root, package)
    path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = path.with_suffix(path.suffix + ".lock")
    with lock_path.open("a+", encoding="utf-8") as lock_handle:
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
        try:
            summary = None
            if path.exists():
                with path.open("r", encoding="utf-8") as handle:
                    summary = json.load(handle)
                if not isinstance(summary, dict) or not isinstance(summary.get("functions"), list):
                    raise ValueError(f"invalid summary structure: {path}")
            elif not create:
                raise FileNotFoundError(f"missing {path}")
            yield path, summary
        finally:
            fcntl.flock(lock_handle.fileno(), fcntl.LOCK_UN)


def new_function(name: str) -> dict[str, Any]:
    func: dict[str, Any] = {
        "name": name,
        "target_order": None,
        "priority_score": None,
        "priority_tier": None,
        "priority_rank": None,
        "priority_reason": "",
        "skip_reason": "",
        "repair_owner": "",
        "error_log": "",
    }
    for stage, spec in STAGES.items():
        field = spec["field"]
        func[field] = "PENDING"
        func[spec["retries"]] = 0
        func[spec["error"]] = ""
        func[spec["fix"]] = ""
        func[f"{field}_started_at"] = ""
        func[f"{field}_completed_at"] = ""
    func["fuzzing_metrics"] = None
    return func


def canonical_name(func: dict[str, Any]) -> str:
    name = str(func.get("name", "")).strip()
    legacy = str(func.get("jni_name", "")).strip()
    if name and legacy and name != legacy:
        raise ValueError(f"conflicting JNI identifiers: {name} != {legacy}")
    if not (name or legacy):
        raise ValueError("summary function is missing name")
    return name or legacy


def normalize_v2(summary: dict[str, Any], package: str) -> dict[str, Any]:
    if summary.get("schema_version") != 2:
        raise ValueError("summary is not schema v2; run migrate first")
    summary["package"] = package
    normalized = []
    for old in summary.get("functions", []):
        name = canonical_name(old)
        defaults = new_function(name)
        defaults.update(old)
        defaults["name"] = name
        defaults.pop("jni_name", None)
        for spec in STAGES.values():
            try:
                defaults[spec["retries"]] = max(0, int(defaults.get(spec["retries"], 0)))
            except (TypeError, ValueError):
                defaults[spec["retries"]] = 0
        normalized.append(defaults)
    summary["functions"] = normalized
    summary["total_jni_functions"] = len(normalized)
    return summary


def find_function(summary: dict[str, Any], jni_name: str) -> dict[str, Any]:
    for func in summary["functions"]:
        if canonical_name(func) == jni_name:
            return func
    raise KeyError(f"JNI not found in summary: {jni_name}")


def priority_tuple(func: dict[str, Any]) -> tuple[int, int, str]:
    rank = func.get("priority_rank")
    order = func.get("target_order")
    return (
        rank if isinstance(rank, int) else 1_000_000,
        order if isinstance(order, int) else 1_000_000,
        canonical_name(func),
    )


def stage_index(stage: str) -> int:
    return STAGE_ORDER.index(stage)


def eligible(func: dict[str, Any], stage: str) -> bool:
    index = stage_index(stage)
    if index == 0:
        return isinstance(func.get("target_order"), int)
    previous = STAGES[STAGE_ORDER[index - 1]]["field"]
    return func.get(previous) == "OK"


def pickable(func: dict[str, Any], stage: str) -> bool:
    if not eligible(func, stage):
        return False
    spec = STAGES[stage]
    status = func.get(spec["field"])
    retries = int(func.get(spec["retries"], 0))
    if retries >= MAX_RETRIES or func.get("repair_owner") == "environment":
        return False
    return status in {"PENDING", "IN_PROGRESS", "FAIL"}


def reset_stage(func: dict[str, Any], stage: str, status: str = "PENDING") -> None:
    spec = STAGES[stage]
    field = spec["field"]
    func[field] = status
    func[f"{field}_started_at"] = ""
    func[f"{field}_completed_at"] = ""
    func[spec["error"]] = ""
    func[spec["fix"]] = ""


def downstream_stages(stage: str) -> tuple[str, ...]:
    return STAGE_ORDER[stage_index(stage) + 1 :]


def propagate_skip(func: dict[str, Any], stage: str) -> None:
    for later in downstream_stages(stage):
        reset_stage(func, later, "SKIP")


def reopen_downstream(func: dict[str, Any], stage: str) -> None:
    for later in downstream_stages(stage):
        spec = STAGES[later]
        if int(func.get(spec["retries"], 0)) >= MAX_RETRIES:
            reset_stage(func, later, "SKIP")
        else:
            reset_stage(func, later, "PENDING")


def snapshot(summary: dict[str, Any]) -> dict[str, Any]:
    remaining = {stage: 0 for stage in SCHEDULING_ORDER}
    environment = 0
    terminal_failures = 0
    skipped = 0
    passed = 0
    for func in summary["functions"]:
        for stage in STAGE_ORDER:
            spec = STAGES[stage]
            status = func.get(spec["field"])
            if pickable(func, stage):
                remaining[stage] += 1
            if status == "ENVIRONMENT":
                environment += 1
            elif status == "FAIL" and not pickable(func, stage):
                terminal_failures += 1
            elif status == "SKIP":
                skipped += 1
            elif status == "OK":
                passed += 1
    return {
        "remaining": remaining,
        "environment_blocked": environment,
        "terminal_failures": terminal_failures,
        "skipped": skipped,
        "passed": passed,
        "all_done": sum(remaining.values()) == 0 and environment == 0,
    }


def next_candidate(summary: dict[str, Any], requested_stage: str | None = None) -> dict[str, Any] | None:
    stages = (requested_stage,) if requested_stage else SCHEDULING_ORDER
    for stage in stages:
        candidates = [func for func in summary["functions"] if pickable(func, stage)]
        if not candidates:
            continue
        candidates.sort(key=priority_tuple)
        func = candidates[0]
        spec = STAGES[stage]
        return {
            "stage": stage,
            "jni_name": canonical_name(func),
            "status": func[spec["field"]],
            "retries": func[spec["retries"]],
            "priority_rank": func.get("priority_rank"),
            "target_order": func.get("target_order"),
        }
    return None


def convert_legacy(summary: dict[str, Any], package: str) -> dict[str, Any]:
    converted: dict[str, Any] = {
        "schema_version": 2,
        "package": package,
        "total_jni_functions": 0,
        "migration": {"source_schema": summary.get("schema_version", 1), "migrated_at": iso_now()},
        "functions": [],
    }
    for old in summary.get("functions", []):
        func = new_function(canonical_name(old))
        for new_key, old_key in (
            ("target_order", "phase2_target_order"),
            ("priority_score", "priority_score"),
            ("priority_tier", "priority_tier"),
            ("priority_rank", "priority_rank"),
            ("priority_reason", "priority_reason"),
            ("error_log", "error_log"),
        ):
            if old_key in old:
                func[new_key] = old[old_key]

        old_call = old.get("phase2_callseq", "PENDING")
        func["call_sequence"] = "OK" if old_call == "OK" else (
            "SKIP" if old_call == "SKIP" else old_call if old_call in {"IN_PROGRESS", "FAIL"} else "PENDING"
        )
        func["call_sequence_started_at"] = old.get("phase2_started_at", "")
        func["call_sequence_last_error"] = old.get("phase2_last_error", "")
        func["skip_reason"] = old.get("phase2_skip_reason", old.get("skip_reason", ""))

        old_build = old.get("phase3_build", "PENDING")
        old_review = old.get("phase3_review", "PENDING")
        old_fuzz = old.get("phase4_fuzz", "PENDING")
        if func["call_sequence"] == "SKIP":
            func["argument_construction"] = "SKIP"
        elif old_build == "OK" and func["call_sequence"] == "OK":
            func["argument_construction"] = "OK"
            func["argument_construction_evidence"] = "legacy_phase3_build"
        elif old_build == "SKIP":
            func["argument_construction"] = "SKIP"

        if old_build == "OK" and (old_review in {"APPROVED", "OK"} or old_fuzz in {"PASS", "OK"}):
            func["harness"] = "OK"
        elif old_build in {"OK", "FAIL", "SKIP", "IN_PROGRESS"}:
            func["harness"] = "FAIL" if old_review in {"REJECTED", "NEEDS_REVISION"} else old_build
        func["harness_retries"] = int(old.get("phase3_retries", 0) or 0)
        func["harness_last_error"] = old.get("phase3_last_error", old.get("phase3_review_issues", ""))
        func["harness_fix_applied"] = old.get("phase3_fix_applied", "")
        func["harness_started_at"] = old.get("phase3_started_at", "")

        fuzz_map = {"PASS": "OK", "OK": "OK", "FAIL": "FAIL", "SKIP": "SKIP", "IN_PROGRESS": "IN_PROGRESS"}
        func["fuzzing"] = fuzz_map.get(old_fuzz, "PENDING")
        func["fuzzing_retries"] = int(old.get("phase4_retries", 0) or 0)
        func["fuzzing_last_error"] = old.get("phase4_last_error", "")
        func["fuzzing_fix_applied"] = old.get("phase4_fix_applied", "")
        func["fuzzing_started_at"] = old.get("phase4_started_at", "")
        func["fuzzing_metrics"] = old.get("phase4_metrics")

        if func["call_sequence"] == "SKIP":
            propagate_skip(func, "call-sequence")
        elif func["argument_construction"] == "SKIP":
            propagate_skip(func, "argument-construction")
        elif func["harness"] == "SKIP":
            propagate_skip(func, "harness")
        converted["functions"].append(func)
    converted["total_jni_functions"] = len(converted["functions"])
    return converted


def cmd_init(root: Path, args: argparse.Namespace) -> int:
    package = safe_package(args.package)
    jni_list_path = Path(args.jni_list).resolve() if args.jni_list else package_dir(root, package) / "jni_list.json"
    with jni_list_path.open("r", encoding="utf-8") as handle:
        jni_list = json.load(handle)
    names = [str(item["jni_name"]) for item in jni_list.get("functions", [])]
    if not names:
        raise ValueError(f"no JNI functions in {jni_list_path}")

    with locked_summary(root, package, create=True) as (path, existing):
        if existing is None:
            summary = {"schema_version": 2, "package": package, "functions": []}
        elif existing.get("schema_version") == 2:
            summary = existing
        else:
            backup = path.with_name("summary.v1.json")
            if not backup.exists():
                shutil.copy2(path, backup)
            summary = convert_legacy(existing, package)
        summary = normalize_v2(summary, package)
        added = []
        for name in names:
            try:
                find_function(summary, name)
            except KeyError:
                summary["functions"].append(new_function(name))
                added.append(name)
        summary["total_jni_functions"] = len(summary["functions"])
        if args.apk:
            summary["apk"] = args.apk
        write_json_atomic(path, summary)
    emit({"ok": True, "package": package, "functions": len(names), "added": added})
    return 0


def migrate_one(root: Path, package: str) -> dict[str, Any]:
    with locked_summary(root, package) as (path, existing):
        assert existing is not None
        if existing.get("schema_version") == 2:
            summary = normalize_v2(existing, package)
            migrated = False
        else:
            backup = path.with_name("summary.v1.json")
            if not backup.exists():
                shutil.copy2(path, backup)
            summary = convert_legacy(existing, package)
            migrated = True
        write_json_atomic(path, summary)
        return {"package": package, "migrated": migrated, "functions": len(summary["functions"])}


def cmd_migrate(root: Path, args: argparse.Namespace) -> int:
    if args.all:
        target_root = root / "target_APK"
        results = [migrate_one(root, path.parent.name) for path in sorted(target_root.glob("*/summary.json"))]
    elif args.package:
        results = [migrate_one(root, safe_package(args.package))]
    else:
        raise ValueError("migrate requires --package or --all")
    emit({"ok": True, "results": results})
    return 0


def cmd_target(root: Path, args: argparse.Namespace) -> int:
    eligible_target = args.eligible == "true"
    if eligible_target and args.order is None:
        raise ValueError("eligible target requires --order")
    with locked_summary(root, args.package) as (path, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        func = find_function(summary, args.jni_name)
        was_filtered = func["call_sequence"] == "SKIP"
        func["target_order"] = args.order if eligible_target else None
        func["skip_reason"] = "" if eligible_target else args.reason
        for key in ("score", "tier", "rank", "priority_reason"):
            value = getattr(args, key)
            if value is not None:
                func[f"priority_{key}" if key != "priority_reason" else key] = value
        if eligible_target and was_filtered:
            for stage in STAGE_ORDER:
                reset_stage(func, stage)
                func[STAGES[stage]["retries"]] = 0
        elif not eligible_target:
            func["call_sequence"] = "SKIP"
            propagate_skip(func, "call-sequence")
        write_json_atomic(path, summary)
    emit({"ok": True, "jni_name": args.jni_name, "eligible": eligible_target})
    return 0


def cmd_next(root: Path, args: argparse.Namespace) -> int:
    stage = normalize_stage(args.stage) if args.stage else None
    with locked_summary(root, args.package) as (_, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        candidate = next_candidate(summary, stage)
        response = {"ok": True, "picked": candidate is not None, "next": candidate}
        response.update(snapshot(summary))
    emit(response)
    return 0


def cmd_start(root: Path, args: argparse.Namespace) -> int:
    stage = normalize_stage(args.stage)
    with locked_summary(root, args.package) as (path, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        func = find_function(summary, args.jni_name)
        if not pickable(func, stage):
            raise ValueError(f"{args.jni_name} is not pickable for {stage}: {func[STAGES[stage]['field']]}")
        spec = STAGES[stage]
        func[spec["field"]] = "IN_PROGRESS"
        func[f"{spec['field']}_started_at"] = iso_now()
        func[f"{spec['field']}_completed_at"] = ""
        write_json_atomic(path, summary)
    emit({"ok": True, "stage": stage, "jni_name": args.jni_name})
    return 0


def cmd_finish(root: Path, args: argparse.Namespace) -> int:
    stage = normalize_stage(args.stage)
    spec = STAGES[stage]
    metrics = None
    if args.metrics:
        metrics = json.loads(args.metrics)
        if not isinstance(metrics, dict):
            raise ValueError("--metrics must be a JSON object")
    with locked_summary(root, args.package) as (path, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        func = find_function(summary, args.jni_name)
        field = spec["field"]
        func[field] = args.result
        func[f"{field}_completed_at"] = iso_now()
        func[spec["error"]] = args.last_error
        func[spec["fix"]] = args.fix_applied
        if metrics is not None:
            func["fuzzing_metrics" if stage == "fuzzing" else f"{field}_metrics"] = metrics

        if args.result == "FAIL":
            if args.consume_retry:
                func[spec["retries"]] = int(func.get(spec["retries"], 0)) + 1
            func["repair_owner"] = stage
            if int(func[spec["retries"]]) >= MAX_RETRIES:
                propagate_skip(func, stage)
        elif args.result == "ENVIRONMENT":
            func["repair_owner"] = "environment"
        elif args.result == "OK":
            func["repair_owner"] = ""
        elif args.result == "SKIP":
            propagate_skip(func, stage)
        write_json_atomic(path, summary)
        response = {"ok": True, "stage": stage, "jni_name": args.jni_name, "result": args.result}
        response.update(snapshot(summary))
        response["next"] = next_candidate(summary)
    emit(response)
    return 0


def cmd_repair(root: Path, args: argparse.Namespace) -> int:
    from_stage = normalize_stage(args.from_stage)
    owner = "environment" if args.owner == "environment" else normalize_stage(args.owner)
    if owner != "environment" and stage_index(owner) > stage_index(from_stage):
        raise ValueError(f"repair owner {owner} is downstream of {from_stage}")
    message = f"{args.category}: {args.error}" if args.error else args.category
    with locked_summary(root, args.package) as (path, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        func = find_function(summary, args.jni_name)
        if owner == "environment":
            spec = STAGES[from_stage]
            func[spec["field"]] = "ENVIRONMENT"
            func[spec["error"]] = message
            func[f"{spec['field']}_completed_at"] = iso_now()
        else:
            spec = STAGES[owner]
            func[spec["field"]] = "FAIL"
            if args.consume_retry:
                func[spec["retries"]] = int(func.get(spec["retries"], 0)) + 1
            func[spec["error"]] = message
            func[f"{spec['field']}_completed_at"] = iso_now()
            if int(func[spec["retries"]]) >= MAX_RETRIES:
                propagate_skip(func, owner)
            else:
                reopen_downstream(func, owner)
        func["repair_owner"] = owner
        write_json_atomic(path, summary)
        response = {"ok": True, "jni_name": args.jni_name, "repair_owner": owner}
        response.update(snapshot(summary))
        response["next"] = next_candidate(summary)
    emit(response)
    return 0


def cmd_resume(root: Path, args: argparse.Namespace) -> int:
    requested_stage = normalize_stage(args.stage) if args.stage else None
    resumed = []
    with locked_summary(root, args.package) as (path, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        functions = [find_function(summary, args.jni_name)] if args.jni_name else summary["functions"]
        for func in functions:
            function_resumed = False
            for stage in STAGE_ORDER:
                if requested_stage and stage != requested_stage:
                    continue
                spec = STAGES[stage]
                if func[spec["field"]] == "ENVIRONMENT":
                    func[spec["field"]] = "PENDING"
                    func[spec["error"]] = ""
                    if args.reset_retries:
                        func[spec["retries"]] = 0
                    function_resumed = True
                    resumed.append({"jni_name": func["name"], "stage": stage})
            if function_resumed and func.get("repair_owner") == "environment":
                func["repair_owner"] = ""
        write_json_atomic(path, summary)
    emit({"ok": True, "resumed": resumed})
    return 0


def cmd_status(root: Path, args: argparse.Namespace) -> int:
    with locked_summary(root, args.package) as (_, summary):
        assert summary is not None
        summary = normalize_v2(summary, args.package)
        response = {"ok": True, "package": args.package}
        response.update(snapshot(summary))
        response["next"] = next_candidate(summary)
    emit(response)
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", help="Omen repository root; defaults to OMEN_ROOT or auto-discovery")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("init")
    p.add_argument("--package", required=True)
    p.add_argument("--jni-list")
    p.add_argument("--apk")
    p.set_defaults(handler=cmd_init)

    p = sub.add_parser("migrate")
    p.add_argument("--package")
    p.add_argument("--all", action="store_true")
    p.set_defaults(handler=cmd_migrate)

    p = sub.add_parser("target")
    p.add_argument("--package", required=True)
    p.add_argument("--jni-name", required=True)
    p.add_argument("--eligible", choices=("true", "false"), required=True)
    p.add_argument("--order", type=int)
    p.add_argument("--reason", default="")
    p.add_argument("--score", type=int)
    p.add_argument("--tier", choices=("HIGH", "MEDIUM", "LOW"))
    p.add_argument("--rank", type=int)
    p.add_argument("--priority-reason", dest="priority_reason")
    p.set_defaults(handler=cmd_target)

    p = sub.add_parser("next")
    p.add_argument("--package", required=True)
    p.add_argument("--stage")
    p.set_defaults(handler=cmd_next)

    p = sub.add_parser("start")
    p.add_argument("--package", required=True)
    p.add_argument("--stage", required=True)
    p.add_argument("--jni-name", required=True)
    p.set_defaults(handler=cmd_start)

    p = sub.add_parser("finish")
    p.add_argument("--package", required=True)
    p.add_argument("--stage", required=True)
    p.add_argument("--jni-name", required=True)
    p.add_argument("--result", choices=("OK", "FAIL", "SKIP", "ENVIRONMENT"), required=True)
    p.add_argument("--last-error", default="")
    p.add_argument("--fix-applied", default="")
    p.add_argument("--metrics")
    p.add_argument("--consume-retry", action=argparse.BooleanOptionalAction, default=True)
    p.set_defaults(handler=cmd_finish)

    p = sub.add_parser("repair")
    p.add_argument("--package", required=True)
    p.add_argument("--jni-name", required=True)
    p.add_argument("--from-stage", required=True)
    p.add_argument("--owner", required=True)
    p.add_argument("--category", required=True)
    p.add_argument("--error", default="")
    p.add_argument("--consume-retry", action=argparse.BooleanOptionalAction, default=True)
    p.set_defaults(handler=cmd_repair)

    p = sub.add_parser("resume")
    p.add_argument("--package", required=True)
    p.add_argument("--jni-name")
    p.add_argument("--stage")
    p.add_argument("--reset-retries", action="store_true")
    p.set_defaults(handler=cmd_resume)

    p = sub.add_parser("status")
    p.add_argument("--package", required=True)
    p.set_defaults(handler=cmd_status)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    root = discover_repo_root(args.repo_root)
    if getattr(args, "package", None) is not None:
        args.package = safe_package(args.package)
    return args.handler(root, args)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print(json.dumps({"ok": False, "error": str(exc)}, ensure_ascii=False), file=sys.stderr)
        raise SystemExit(1)
