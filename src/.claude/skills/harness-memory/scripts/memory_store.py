#!/usr/bin/env python3
"""Store and retrieve verified, package-local Harness Memory components."""

from __future__ import annotations

import argparse
import copy
import fcntl
import hashlib
import json
import os
import shutil
import sys
import tempfile
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path


SCHEMA_VERSION = 1
KINDS = ("call-sequence", "argument-recipe")
COLLECTIONS = {
    "call-sequence": "call_sequences",
    "argument-recipe": "argument_recipes",
}
REQUIRED_VERIFICATION = {
    "call-sequence": "GLOBAL_REVIEW_ACCEPT",
    "argument-recipe": "SYMBOL_CHECK+INDEPENDENT_BUILD+DEVICE_SMOKE_PASS",
}
REQUIRED_ARTIFACTS = {
    "call-sequence": {"call_sequence.md"},
    "argument-recipe": {"construct.c", "recipe.md"},
}
RESERVED_ARTIFACT_NAMES = {"metadata.json", "validation.json"}


class MemoryStoreError(Exception):
    """An expected CLI or storage error."""


def canonical_json(value):
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    )


def reject_json_constant(value):
    raise ValueError("non-finite number %s is not valid JSON" % value)


def parse_json_object(text, description):
    try:
        value = json.loads(text, parse_constant=reject_json_constant)
    except (json.JSONDecodeError, ValueError) as exc:
        raise MemoryStoreError("%s is not valid JSON: %s" % (description, exc)) from exc
    if not isinstance(value, dict):
        raise MemoryStoreError("%s must be a JSON object" % description)
    try:
        canonical_json(value)
    except (TypeError, ValueError) as exc:
        raise MemoryStoreError("%s is not canonicalizable JSON: %s" % (description, exc)) from exc
    return value


def read_json_object(path, description):
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise MemoryStoreError("cannot read %s %s: %s" % (description, path, exc)) from exc
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise MemoryStoreError("%s %s is not UTF-8" % (description, path)) from exc
    return raw, parse_json_object(text, description)


def validate_path_component(value, description):
    if (
        not value
        or value in (".", "..")
        or "\x00" in value
        or "/" in value
        or "\\" in value
        or Path(value).is_absolute()
    ):
        raise MemoryStoreError("unsafe %s: %r" % (description, value))


def iter_ancestors(start):
    yield start
    yield from start.parents


def discover_repo_root():
    starts = (Path.cwd(), Path(__file__).resolve().parent)
    for start in starts:
        for ancestor in iter_ancestors(start.resolve()):
            if (ancestor / "target_APK").is_dir():
                return ancestor
            if (ancestor / ".agents" / "skills" / "harness-memory").is_dir():
                return ancestor
    raise MemoryStoreError(
        "cannot discover repository root; set OMEN_ROOT or pass --repo-root"
    )


def resolve_repo_root(cli_root):
    if cli_root:
        root = Path(cli_root).expanduser().resolve()
    elif os.environ.get("OMEN_ROOT"):
        root = Path(os.environ["OMEN_ROOT"]).expanduser().resolve()
    else:
        root = discover_repo_root()
    if not root.is_dir():
        raise MemoryStoreError("repository root is not a directory: %s" % root)
    return root


def component_digest(name, compatibility):
    payload = name + canonical_json(compatibility)
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def load_validation(path, kind):
    raw, validation = read_json_object(path, "validation JSON")
    expected = REQUIRED_VERIFICATION[kind]
    if validation.get("verification") != expected:
        raise MemoryStoreError(
            "%s requires verification=%s" % (kind, expected)
        )
    return raw, validation


def load_artifacts(paths, kind):
    artifacts = []
    seen = set()
    for value in paths:
        source = Path(value).expanduser()
        if not source.is_file():
            raise MemoryStoreError("artifact is not a regular file: %s" % source)
        filename = source.name
        if filename in RESERVED_ARTIFACT_NAMES:
            raise MemoryStoreError("artifact uses a reserved filename: %s" % filename)
        if filename in seen:
            raise MemoryStoreError("duplicate artifact filename: %s" % filename)
        seen.add(filename)
        try:
            data = source.read_bytes()
        except OSError as exc:
            raise MemoryStoreError("cannot read artifact %s: %s" % (source, exc)) from exc
        artifacts.append(
            {"path": filename, "sha256": sha256_bytes(data), "data": data}
        )

    missing = REQUIRED_ARTIFACTS[kind] - seen
    if missing:
        raise MemoryStoreError(
            "%s is missing required artifact(s): %s"
            % (kind, ", ".join(sorted(missing)))
        )
    artifacts.sort(key=lambda item: item["path"])
    return artifacts


def utc_timestamp():
    return datetime.now(timezone.utc).isoformat(timespec="seconds").replace(
        "+00:00", "Z"
    )


def empty_index(package):
    return {"schema_version": SCHEMA_VERSION, "package": package, "entries": []}


def load_index(memory_dir, package):
    path = memory_dir / "index.json"
    if not path.exists():
        return empty_index(package)
    _, index = read_json_object(path, "memory index")
    if index.get("schema_version") != SCHEMA_VERSION:
        raise MemoryStoreError("unsupported memory index schema in %s" % path)
    if index.get("package") != package:
        raise MemoryStoreError("memory index package mismatch in %s" % path)
    if not isinstance(index.get("entries"), list) or not all(
        isinstance(entry, dict) for entry in index["entries"]
    ):
        raise MemoryStoreError("memory index entries must be an array of objects")
    return index


@contextmanager
def exclusive_lock(path):
    with path.open("a+b") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)


def atomic_write_bytes(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary_name = tempfile.mkstemp(
        prefix=".%s." % path.name, suffix=".tmp", dir=str(path.parent)
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(fd, "wb") as output:
            output.write(data)
            output.flush()
            os.fsync(output.fileno())
        os.replace(str(temporary), str(path))
    finally:
        try:
            temporary.unlink()
        except FileNotFoundError:
            pass


def json_bytes(value):
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2, sort_keys=True)
        + "\n"
    ).encode("utf-8")


def package_paths(repo_root, package):
    package_dir = repo_root / "target_APK" / package
    return package_dir, package_dir / "memory"


def component_directory(memory_dir, kind, name, digest):
    return memory_dir / COLLECTIONS[kind] / name / digest


def resolve_package_relative(package_dir, value):
    if not isinstance(value, str) or not value:
        raise MemoryStoreError("invalid artifact path in memory index")
    relative = Path(value)
    if relative.is_absolute() or ".." in relative.parts:
        raise MemoryStoreError("unsafe artifact path in memory index: %s" % value)
    package_resolved = package_dir.resolve()
    resolved = (package_dir / relative).resolve()
    try:
        resolved.relative_to(package_resolved)
    except ValueError as exc:
        raise MemoryStoreError("artifact path escapes package: %s" % value) from exc
    return resolved


def public_entry(entry, package_dir):
    result = copy.deepcopy(entry)
    result["artifacts"] = []
    for artifact in entry.get("artifacts", []):
        if not isinstance(artifact, dict) or "path" not in artifact:
            raise MemoryStoreError("invalid artifact record in memory index")
        converted = copy.deepcopy(artifact)
        converted["path"] = str(resolve_package_relative(package_dir, artifact["path"]))
        result["artifacts"].append(converted)
    return result


def make_records(repo_root, package, kind, name, digest, compatibility, validation,
                 source_jni, artifacts, created_at):
    relative_dir = Path("memory") / COLLECTIONS[kind] / name / digest
    common = {
        "schema_version": SCHEMA_VERSION,
        "kind": kind,
        "name": name,
        "digest": digest,
        "compatibility": compatibility,
        "verification": validation["verification"],
        "source_jni": source_jni,
        "created_at": created_at,
    }
    metadata = copy.deepcopy(common)
    metadata["artifacts"] = [
        {"path": artifact["path"], "sha256": artifact["sha256"]}
        for artifact in artifacts
    ]
    entry = copy.deepcopy(common)
    entry["artifacts"] = [
        {
            "path": (relative_dir / artifact["path"]).as_posix(),
            "sha256": artifact["sha256"],
        }
        for artifact in artifacts
    ]
    return metadata, entry


def existing_content_matches(entry, metadata, entry_artifacts, validation, artifacts,
                             package_dir, component_dir):
    ignored = {"created_at", "artifacts"}
    if any(entry.get(key) != value for key, value in metadata.items() if key not in ignored):
        return False
    if entry.get("artifacts") != entry_artifacts:
        return False
    try:
        _, stored_validation = read_json_object(
            component_dir / "validation.json", "stored validation JSON"
        )
        _, stored_metadata = read_json_object(
            component_dir / "metadata.json", "stored metadata JSON"
        )
    except MemoryStoreError:
        return False
    if stored_validation != validation:
        return False
    if any(
        stored_metadata.get(key) != value
        for key, value in metadata.items()
        if key != "created_at"
    ):
        return False
    for requested in artifacts:
        stored_path = (Path("memory") / COLLECTIONS[metadata["kind"]] /
                       metadata["name"] / metadata["digest"] / requested["path"])
        try:
            actual_path = resolve_package_relative(package_dir, stored_path.as_posix())
            actual = actual_path.read_bytes()
        except (MemoryStoreError, OSError):
            return False
        if sha256_bytes(actual) != requested["sha256"]:
            return False
    return True


def put_component(repo_root, package, kind, name, compatibility, validation_path,
                  source_jni, artifact_paths):
    validate_path_component(package, "package")
    validate_path_component(name, "name")
    if not source_jni:
        raise MemoryStoreError("--source-jni must not be empty")
    validation_bytes, validation = load_validation(Path(validation_path).expanduser(), kind)
    artifacts = load_artifacts(artifact_paths, kind)
    digest = component_digest(name, compatibility)
    package_dir, memory_dir = package_paths(repo_root, package)
    memory_dir.mkdir(parents=True, exist_ok=True)

    with exclusive_lock(memory_dir / "index.lock"):
        index = load_index(memory_dir, package)
        existing = next(
            (
                entry
                for entry in index["entries"]
                if entry.get("kind") == kind
                and entry.get("name") == name
                and entry.get("digest") == digest
            ),
            None,
        )
        component_dir = component_directory(memory_dir, kind, name, digest)

        if existing is not None:
            metadata, expected_entry = make_records(
                repo_root, package, kind, name, digest, compatibility, validation,
                source_jni, artifacts, existing.get("created_at", "")
            )
            if existing_content_matches(
                existing, metadata, expected_entry["artifacts"], validation, artifacts,
                package_dir, component_dir
            ):
                return public_entry(existing, package_dir)
            raise MemoryStoreError(
                "refusing to overwrite existing component with different content: %s" % digest
            )

        if component_dir.exists():
            raise MemoryStoreError(
                "component directory exists without an index entry: %s" % component_dir
            )

        metadata, entry = make_records(
            repo_root, package, kind, name, digest, compatibility, validation,
            source_jni, artifacts, utc_timestamp()
        )
        staging = Path(tempfile.mkdtemp(prefix=".put-", dir=str(memory_dir)))
        installed = False
        try:
            for artifact in artifacts:
                (staging / artifact["path"]).write_bytes(artifact["data"])
            (staging / "validation.json").write_bytes(validation_bytes)
            (staging / "metadata.json").write_bytes(json_bytes(metadata))
            component_dir.parent.mkdir(parents=True, exist_ok=True)
            staging.rename(component_dir)
            installed = True
            index["entries"].append(entry)
            index["entries"].sort(
                key=lambda item: (
                    item.get("kind", ""), item.get("name", ""), item.get("digest", "")
                )
            )
            atomic_write_bytes(memory_dir / "index.json", json_bytes(index))
        except Exception:
            if installed:
                shutil.rmtree(component_dir, ignore_errors=True)
            raise
        finally:
            if staging.exists():
                shutil.rmtree(staging, ignore_errors=True)

    return public_entry(entry, package_dir)


def query_components(repo_root, package, kind, name):
    validate_path_component(package, "package")
    validate_path_component(name, "name")
    package_dir, memory_dir = package_paths(repo_root, package)
    index = load_index(memory_dir, package)
    entries = [
        entry
        for entry in index["entries"]
        if entry.get("kind") == kind and entry.get("name") == name
    ]
    return [public_entry(entry, package_dir) for entry in entries]


def get_component(repo_root, package, kind, name, compatibility):
    validate_path_component(package, "package")
    validate_path_component(name, "name")
    digest = component_digest(name, compatibility)
    package_dir, memory_dir = package_paths(repo_root, package)
    index = load_index(memory_dir, package)
    for entry in index["entries"]:
        if (
            entry.get("kind") == kind
            and entry.get("name") == name
            and entry.get("digest") == digest
            and entry.get("compatibility") == compatibility
        ):
            return public_entry(entry, package_dir)
    raise MemoryStoreError("component not found: %s %s %s" % (kind, name, digest))


def build_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", help="repository root (overrides OMEN_ROOT)")
    subparsers = parser.add_subparsers(dest="command", required=True)

    def add_common(subparser):
        subparser.add_argument("--package", required=True)
        subparser.add_argument("--kind", required=True, choices=KINDS)
        subparser.add_argument("--name", required=True)

    query = subparsers.add_parser("query", help="list candidates by kind and name")
    add_common(query)

    get = subparsers.add_parser("get", help="get one exact compatibility key")
    add_common(get)
    get.add_argument("--compatibility", required=True)

    put = subparsers.add_parser("put", help="store a verified component")
    add_common(put)
    put.add_argument("--compatibility", required=True)
    put.add_argument("--validation", required=True)
    put.add_argument("--source-jni", required=True)
    put.add_argument("--artifact", required=True, action="append")
    return parser


def print_result(value):
    print(json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2, sort_keys=True))


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        repo_root = resolve_repo_root(args.repo_root)
        if args.command == "query":
            result = query_components(repo_root, args.package, args.kind, args.name)
        elif args.command == "get":
            compatibility = parse_json_object(args.compatibility, "--compatibility")
            result = get_component(
                repo_root, args.package, args.kind, args.name, compatibility
            )
        else:
            compatibility = parse_json_object(args.compatibility, "--compatibility")
            result = put_component(
                repo_root,
                args.package,
                args.kind,
                args.name,
                compatibility,
                args.validation,
                args.source_jni,
                args.artifact,
            )
        print_result(result)
        return 0
    except (MemoryStoreError, OSError) as exc:
        print("memory_store: error: %s" % exc, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
