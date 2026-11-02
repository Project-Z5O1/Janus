#!/usr/bin/env python3
"""Convert jniinfo.py extract output into an auditable JNI Binding Table."""
from __future__ import annotations

import argparse
import json
import os
import tempfile
from pathlib import Path
from typing import Any


def write_json_atomic(path: Path, data: dict[str, Any]) -> None:
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


def normalize_offset(value: Any) -> str:
    if isinstance(value, int):
        return hex(value)
    if isinstance(value, str):
        return hex(int(value, 0))
    raise ValueError(f"invalid JNI offset: {value!r}")


def build_table(extract: dict[str, Any], package: str) -> dict[str, Any]:
    if not extract.get("ok"):
        raise ValueError(extract.get("error", "JNI extraction did not succeed"))
    entries = extract.get("entries")
    if not isinstance(entries, list) or not entries:
        raise ValueError("JNI extraction returned no entries")

    functions: list[dict[str, Any]] = []
    by_signature: dict[tuple[str, str, str, str], dict[str, Any]] = {}
    seen_identities: set[tuple[str, str, str]] = set()
    for entry in entries:
        if entry.get("error"):
            raise ValueError(f"unresolved binding entry: {entry.get('raw', entry['error'])}")
        jni_name = str(entry["jni_name"])
        java_class = str(entry.get("java_class") or "")
        java_method = str(entry.get("java_method") or "")
        params = ",".join(str(item) for item in entry.get("param_types", []))
        return_type = str(entry.get("return_type") or "")
        library = str(entry["library"])
        offset = normalize_offset(entry["offset"])
        if not java_class or not java_method or not return_type:
            raise ValueError(f"incomplete Java binding for {jni_name}")

        signature = (java_class, java_method, params, return_type)
        identity = (jni_name, library, offset)
        if signature in by_signature:
            previous = by_signature[signature]
            previous_identity = (
                previous["jni_name"],
                previous["library"],
                previous["offset"],
            )
            if previous_identity != identity:
                raise ValueError(
                    "conflicting bindings for "
                    f"{java_class}.{java_method}({params}){return_type}: "
                    f"{previous_identity} vs {identity}"
                )
            continue
        if identity in seen_identities:
            continue

        record = {
            "jni_name": jni_name,
            "java_class": java_class,
            "java_method": java_method,
            "params": params,
            "return_type": return_type,
            "library": library,
            "offset": offset,
            "binding_kind": str(entry.get("binding_kind") or "device_resolved"),
            "binding_evidence": str(entry.get("binding_evidence") or entry.get("raw") or ""),
        }
        by_signature[signature] = record
        seen_identities.add(identity)
        functions.append(record)

    return {"package": package, "total_jni_functions": len(functions), "functions": functions}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--extract-result", required=True, type=Path)
    parser.add_argument("--package", required=True)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    with args.extract_result.open("r", encoding="utf-8") as handle:
        extract = json.load(handle)
    table = build_table(extract, args.package)
    write_json_atomic(args.output, table)
    print(json.dumps({"ok": True, "functions": len(table["functions"]), "output": str(args.output)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
