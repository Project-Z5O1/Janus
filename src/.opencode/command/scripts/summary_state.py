#!/usr/bin/env python3
"""Serial summary.json state helper.

Phase 2/3/3.5/4 are executed strictly serially per JNI, so this helper exposes
only three operations:

  - next   : read-only pick of the highest-priority eligible JNI for a phase
  - start  : flip status to IN_PROGRESS (debug breadcrumb; safe across crashes)
  - finish : write the terminal result (OK/FAIL/SKIP/PASS/APPROVED/...)

It uses an exclusive file lock + atomic JSON write so that the package-level
Phase 1.5 / 1.75 gates and the per-JNI Phase 2/3/3.5/4 transitions never
interleave a partial write.

Legacy summary.json files written under the old concurrent claim/lease schema
are normalized on read by pruning deprecated owner/lease/attempt fields.
"""
from __future__ import annotations

import argparse
import contextlib
import fcntl
import json
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterator


REPO_ROOT = Path(__file__).resolve().parents[3]
TARGET_APK_DIR = REPO_ROOT / "target_APK"
MAX_RETRIES = 3

PHASE_SPECS: dict[str, dict[str, Any]] = {
    "phase2": {
        "status_field": "phase2_callseq",
        "started_field": "phase2_started_at",
        "allowed_results": {"OK", "SKIP"},
        "retry_field": None,
        "error_field": None,
        "fix_field": None,
    },
    "phase3": {
        "status_field": "phase3_build",
        "started_field": "phase3_started_at",
        "allowed_results": {"OK", "FAIL", "SKIP"},
        "retry_field": "phase3_retries",
        "error_field": "phase3_last_error",
        "fix_field": "phase3_fix_applied",
    },
    "phase3_review": {
        "status_field": "phase3_review",
        "started_field": "phase3_review_started_at",
        "allowed_results": {"APPROVED", "NEEDS_REVISION", "REJECTED", "SKIP"},
        "retry_field": None,
        "error_field": "phase3_review_issues",
        "fix_field": None,
    },
    "phase4": {
        "status_field": "phase4_fuzz",
        "started_field": "phase4_started_at",
        "allowed_results": {"PASS", "FAIL", "SKIP"},
        "retry_field": "phase4_retries",
        "error_field": "phase4_last_error",
        "fix_field": "phase4_fix_applied",
    },
}

# Fields written by the previous concurrent design. Pruned on read so older
# summary.json files migrate forward automatically.
DEPRECATED_PHASE_FIELDS = (
    "phase2_owner", "phase2_lease_until", "phase2_attempt_id",
    "phase3_owner", "phase3_lease_until", "phase3_attempt_id",
    "phase3_review_owner", "phase3_review_lease_until", "phase3_review_attempt_id",
    "phase4_owner", "phase4_lease_until", "phase4_attempt_id",
)


def now_utc() -> datetime:
    return datetime.now(timezone.utc)


def iso_utc(dt: datetime) -> str:
    return (
        dt.astimezone(timezone.utc)
        .replace(microsecond=0)
        .isoformat()
        .replace("+00:00", "Z")
    )


def write_json_atomic(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w",
        encoding="utf-8",
        dir=path.parent,
        prefix=f".{path.name}.",
        delete=False,
    ) as handle:
        json.dump(data, handle, ensure_ascii=True, indent=2)
        handle.write("\n")
        temp_path = Path(handle.name)
    temp_path.replace(path)


def normalize_phase(phase_arg: str) -> str:
    mapping = {
        "phase2": "phase2",
        "phase2_callseq": "phase2",
        "phase3": "phase3",
        "phase3_build": "phase3",
        "phase3_review": "phase3_review",
        "phase4": "phase4",
        "phase4_fuzz": "phase4",
    }
    try:
        return mapping[phase_arg]
    except KeyError as exc:
        raise argparse.ArgumentTypeError(f"unsupported phase: {phase_arg}") from exc


def summary_path_for_package(package: str) -> Path:
    return TARGET_APK_DIR / package / "summary.json"


@contextlib.contextmanager
def locked_summary(package: str) -> Iterator[tuple[Path, dict[str, Any]]]:
    summary_path = summary_path_for_package(package)
    if not summary_path.exists():
        raise FileNotFoundError(f"missing {summary_path}")

    lock_path = summary_path.with_suffix(summary_path.suffix + ".lock")
    with lock_path.open("a+", encoding="utf-8") as lock_handle:
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
        with summary_path.open("r", encoding="utf-8") as handle:
            summary = json.load(handle)
        summary = normalize_summary_schema(summary)
        yield summary_path, summary
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_UN)


def normalize_summary_schema(summary: dict[str, Any]) -> dict[str, Any]:
    for func in summary.get("functions", []):
        canonical_name = canonical_jni_name(func)
        if not canonical_name:
            raise KeyError(
                "missing function identifier in summary entry: expected name"
            )
        func["name"] = canonical_name

        # Prune fields from the legacy concurrent schema.
        for legacy in DEPRECATED_PHASE_FIELDS:
            func.pop(legacy, None)

        for phase_name, spec in PHASE_SPECS.items():
            func.setdefault(spec["status_field"], "PENDING")
            func.setdefault(spec["started_field"], "")

            if spec["retry_field"]:
                func.setdefault(spec["retry_field"], 0)
            if spec["error_field"]:
                func.setdefault(spec["error_field"], "")
            if spec["fix_field"]:
                func.setdefault(spec["fix_field"], "")

            if phase_name == "phase3_review":
                func.setdefault("phase3_review_revision_count", 0)
            if phase_name == "phase4":
                func.setdefault("phase4_metrics", None)
    return normalize_downstream_states(summary)


def priority_tuple(func: dict[str, Any]) -> tuple[int, int, str]:
    rank = func.get("priority_rank")
    order = func.get("phase2_target_order")
    rank_key = int(rank) if isinstance(rank, int) else 1_000_000
    order_key = int(order) if isinstance(order, int) else 1_000_000
    return (rank_key, order_key, canonical_jni_name(func))


def canonical_jni_name(func: dict[str, Any]) -> str:
    name = str(func.get("name", "")).strip()
    legacy_name = str(func.get("jni_name", "")).strip()
    if name and legacy_name and name != legacy_name:
        raise KeyError(
            f"conflicting function identifiers in summary entry: name={name} jni_name={legacy_name}"
        )
    return name or legacy_name


def retry_count(func: dict[str, Any], spec: dict[str, Any]) -> int:
    retry_field = spec["retry_field"]
    if not retry_field:
        return 0
    try:
        return int(func.get(retry_field, 0))
    except (TypeError, ValueError):
        return 0


def phase_eligible(func: dict[str, Any], phase_name: str) -> bool:
    if phase_name == "phase2":
        return func.get("phase2_target_order") is not None
    if phase_name == "phase3":
        return func.get("phase2_callseq") == "OK"
    if phase_name == "phase3_review":
        return func.get("phase3_build") == "OK"
    if phase_name == "phase4":
        return (
            func.get("phase3_build") == "OK"
            and func.get("phase3_review") == "APPROVED"
        )
    return False


def status_pickable(func: dict[str, Any], phase_name: str) -> bool:
    """Return True if a phase status can be picked up by `next`/`start`.

    Under serial execution we accept:
      * PENDING                    : never started
      * IN_PROGRESS                : crash residue from a previous run
      * FAIL with retries left     : retryable failure (phase3 / phase4 only)
    """
    spec = PHASE_SPECS[phase_name]
    status = func.get(spec["status_field"])
    if status in {"PENDING", "IN_PROGRESS"}:
        return True
    if (
        status == "FAIL"
        and spec["retry_field"]
        and retry_count(func, spec) < MAX_RETRIES
    ):
        return True
    return False


def reset_phase_state(func: dict[str, Any], phase_name: str) -> None:
    """Reset the running breadcrumb (started_at) for a phase."""
    spec = PHASE_SPECS[phase_name]
    func[spec["started_field"]] = ""


def propagate_downstream_skip(func: dict[str, Any], phase_name: str) -> None:
    if phase_name == "phase2":
        if func.get("phase3_build") in {"PENDING", "IN_PROGRESS"}:
            func["phase3_build"] = "SKIP"
            reset_phase_state(func, "phase3")
        if func.get("phase3_review") in {"PENDING", "IN_PROGRESS"}:
            func["phase3_review"] = "SKIP"
            reset_phase_state(func, "phase3_review")
        if func.get("phase4_fuzz") in {"PENDING", "IN_PROGRESS"}:
            func["phase4_fuzz"] = "SKIP"
            reset_phase_state(func, "phase4")
    elif phase_name == "phase3":
        if func.get("phase3_review") in {"PENDING", "IN_PROGRESS"}:
            func["phase3_review"] = "SKIP"
            reset_phase_state(func, "phase3_review")
        if func.get("phase4_fuzz") in {"PENDING", "IN_PROGRESS"}:
            func["phase4_fuzz"] = "SKIP"
            reset_phase_state(func, "phase4")
    elif phase_name == "phase3_review":
        if func.get("phase4_fuzz") in {"PENDING", "IN_PROGRESS"}:
            func["phase4_fuzz"] = "SKIP"
            reset_phase_state(func, "phase4")


def normalize_downstream_states(summary: dict[str, Any]) -> dict[str, Any]:
    for func in summary.get("functions", []):
        if func.get("phase2_callseq") == "SKIP":
            propagate_downstream_skip(func, "phase2")
            continue

        if func.get("phase3_build") == "SKIP":
            propagate_downstream_skip(func, "phase3")
            continue

        if (
            func.get("phase3_build") == "FAIL"
            and retry_count(func, PHASE_SPECS["phase3"]) >= MAX_RETRIES
        ):
            propagate_downstream_skip(func, "phase3")
            continue

        if func.get("phase3_review") in {"NEEDS_REVISION", "REJECTED"}:
            if retry_count(func, PHASE_SPECS["phase3"]) >= MAX_RETRIES:
                propagate_downstream_skip(func, "phase3_review")

    return summary


def find_function(summary: dict[str, Any], jni_name: str) -> dict[str, Any]:
    for func in summary.get("functions", []):
        if canonical_jni_name(func) == jni_name:
            return func
    raise KeyError(f"jni not found in summary: {jni_name}")


# Phase scheduling priority used by the orchestrator: drain later phases first
# so in-flight JNIs converge to a terminal state quickly.
SCHEDULING_ORDER = ("phase4", "phase3_review", "phase3", "phase2")


def remaining_counts(summary: dict[str, Any]) -> dict[str, int]:
    counts: dict[str, int] = {}
    for phase_name in SCHEDULING_ORDER:
        n = 0
        for func in summary.get("functions", []):
            if phase_eligible(func, phase_name) and status_pickable(func, phase_name):
                n += 1
        counts[phase_name] = n
    return counts


def pick_next_global(summary: dict[str, Any]) -> dict[str, Any] | None:
    """Return the next JNI to dispatch across all phases, or None if drained."""
    for phase_name in SCHEDULING_ORDER:
        eligible = [
            func
            for func in summary.get("functions", [])
            if phase_eligible(func, phase_name)
            and status_pickable(func, phase_name)
        ]
        if not eligible:
            continue
        eligible.sort(key=priority_tuple)
        func = eligible[0]
        spec = PHASE_SPECS[phase_name]
        preview = {
            "phase": phase_name,
            "jni_name": canonical_jni_name(func),
            "status": func.get(spec["status_field"]),
            "priority_rank": func.get("priority_rank"),
            "phase2_target_order": func.get("phase2_target_order"),
        }
        if spec["retry_field"]:
            preview["retries"] = retry_count(func, spec)
        return preview
    return None


def cmd_next(args: argparse.Namespace) -> int:
    """Read-only pick of the highest-priority eligible JNI for a phase."""
    phase_name = normalize_phase(args.phase)

    with locked_summary(args.package) as (summary_path, summary):
        eligible = [
            func
            for func in summary.get("functions", [])
            if phase_eligible(func, phase_name)
            and status_pickable(func, phase_name)
        ]
        eligible.sort(key=priority_tuple)

        # Persist any schema migration done by normalize_summary_schema
        # (e.g. pruning legacy owner/lease/attempt fields) even when no
        # candidate is returned.
        write_json_atomic(summary_path, summary)

        if not eligible:
            print(json.dumps({"ok": True, "picked": False}, ensure_ascii=True))
            return 0

        func = eligible[0]
        spec = PHASE_SPECS[phase_name]
        response = {
            "ok": True,
            "picked": True,
            "phase": phase_name,
            "jni_name": canonical_jni_name(func),
            "status": func.get(spec["status_field"]),
            "priority_rank": func.get("priority_rank"),
            "phase2_target_order": func.get("phase2_target_order"),
        }
        if spec["retry_field"]:
            response["retries"] = retry_count(func, spec)
        print(json.dumps(response, ensure_ascii=True))
        return 0


def cmd_start(args: argparse.Namespace) -> int:
    """Mark a JNI's phase as IN_PROGRESS (debug breadcrumb)."""
    phase_name = normalize_phase(args.phase)
    spec = PHASE_SPECS[phase_name]
    ref_time = now_utc()

    with locked_summary(args.package) as (summary_path, summary):
        func = find_function(summary, args.jni_name)
        if not phase_eligible(func, phase_name):
            print(json.dumps(
                {"ok": False, "error": f"jni {args.jni_name} not eligible for {phase_name}"},
                ensure_ascii=True,
            ))
            return 1
        if not status_pickable(func, phase_name):
            print(json.dumps(
                {
                    "ok": False,
                    "error": f"jni {args.jni_name} not pickable; status={func.get(spec['status_field'])}",
                },
                ensure_ascii=True,
            ))
            return 1
        func[spec["status_field"]] = "IN_PROGRESS"
        func[spec["started_field"]] = iso_utc(ref_time)
        write_json_atomic(summary_path, summary)
        print(json.dumps(
            {"ok": True, "phase": phase_name, "jni_name": args.jni_name},
            ensure_ascii=True,
        ))
        return 0


def cmd_finish(args: argparse.Namespace) -> int:
    """Write the terminal phase result and propagate any downstream SKIPs."""
    phase_name = normalize_phase(args.phase)
    spec = PHASE_SPECS[phase_name]
    result = args.result
    if result not in spec["allowed_results"]:
        print(json.dumps(
            {"ok": False, "error": f"invalid result for {phase_name}: {result}"},
            ensure_ascii=True,
        ))
        return 1

    with locked_summary(args.package) as (summary_path, summary):
        func = find_function(summary, args.jni_name)
        func[spec["status_field"]] = result

        if spec["retry_field"] and result == "FAIL":
            func[spec["retry_field"]] = retry_count(func, spec) + 1
        if spec["error_field"]:
            func[spec["error_field"]] = args.last_error or ""
        if spec["fix_field"]:
            func[spec["fix_field"]] = args.fix_applied or ""

        if args.metrics:
            try:
                metrics_obj = json.loads(args.metrics)
                if isinstance(metrics_obj, dict):
                    func[f"{phase_name}_metrics"] = metrics_obj
            except json.JSONDecodeError:
                pass  # malformed metrics should not fail the finish call

        # When phase3 succeeds again after a review rejection, re-open review.
        if (
            phase_name == "phase3"
            and result == "OK"
            and func.get("phase3_review") in {"NEEDS_REVISION", "REJECTED"}
        ):
            func["phase3_review"] = "PENDING"
            reset_phase_state(func, "phase3_review")

        if result == "SKIP":
            propagate_downstream_skip(func, phase_name)
        elif (
            phase_name == "phase3"
            and result == "FAIL"
            and retry_count(func, spec) >= MAX_RETRIES
        ):
            propagate_downstream_skip(func, phase_name)
        elif phase_name == "phase3_review" and result in {"NEEDS_REVISION", "REJECTED"}:
            func["phase3_review_revision_count"] = (
                func.get("phase3_review_revision_count", 0) + 1
            )
            phase3_spec = PHASE_SPECS["phase3"]
            func["phase3_build"] = "FAIL"
            func["phase3_retries"] = retry_count(func, phase3_spec) + 1
            reset_phase_state(func, "phase3")
            if func["phase3_retries"] >= MAX_RETRIES:
                propagate_downstream_skip(func, "phase3")

        write_json_atomic(summary_path, summary)
        response = {
            "ok": True,
            "phase": phase_name,
            "jni_name": args.jni_name,
            "result": result,
            "next": pick_next_global(summary),
            "remaining": remaining_counts(summary),
        }
        if response["next"] is None:
            response["all_done"] = True
        print(json.dumps(response, ensure_ascii=True))
        return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Serial summary.json state helper (next/start/finish)."
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p_next = sub.add_parser("next", help="Return the next eligible JNI for a phase.")
    p_next.add_argument("--package", required=True)
    p_next.add_argument("--phase", required=True)
    p_next.set_defaults(func=cmd_next)

    p_start = sub.add_parser("start", help="Mark a JNI's phase as IN_PROGRESS.")
    p_start.add_argument("--package", required=True)
    p_start.add_argument("--phase", required=True)
    p_start.add_argument("--jni-name", required=True)
    p_start.set_defaults(func=cmd_start)

    p_finish = sub.add_parser("finish", help="Write the terminal phase result.")
    p_finish.add_argument("--package", required=True)
    p_finish.add_argument("--phase", required=True)
    p_finish.add_argument("--jni-name", required=True)
    p_finish.add_argument("--result", required=True)
    p_finish.add_argument("--last-error", default="")
    p_finish.add_argument("--fix-applied", default="")
    p_finish.add_argument(
        "--metrics",
        default="",
        help="JSON dict of phase metrics (currently used by phase4)",
    )
    p_finish.set_defaults(func=cmd_finish)

    return parser


def main(argv: list[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except (FileNotFoundError, KeyError) as exc:
        print(json.dumps({"ok": False, "error": str(exc)}, ensure_ascii=True), file=sys.stderr)
        raise SystemExit(1)
