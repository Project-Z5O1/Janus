#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import re


DEVICE_ROOT = "/data/local/tmp/omen/recipes"


def sanitize_component(value: str, max_len: int) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9._-]+", "_", (value or "").strip())
    cleaned = re.sub(r"_+", "_", cleaned).strip("._-")
    if not cleaned:
        cleaned = "unnamed"
    return cleaned[:max_len]


def build_recipe_device_dir(package: str, jni_name: str, class_name: str) -> str:
    package_part = sanitize_component(package, 48)
    jni_part = sanitize_component(jni_name, 64)
    class_part = sanitize_component(class_name, 48)
    digest_src = f"{package}\n{jni_name}\n{class_name}".encode("utf-8")
    digest = hashlib.sha256(digest_src).hexdigest()[:12]
    return f"{DEVICE_ROOT}/{package_part}/{jni_part}/{class_part}_{digest}"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Build a namespaced device dir for recipe verification.")
    parser.add_argument("--package", required=True, help="Android package name")
    parser.add_argument("--jni-name", required=True, help="JNI function name")
    parser.add_argument("--class-name", required=True, help="Java class name for the recipe")
    parser.add_argument(
        "--format",
        choices=("path", "json", "shell"),
        default="path",
        help="Output format",
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    device_dir = build_recipe_device_dir(args.package, args.jni_name, args.class_name)

    if args.format == "json":
        print(
            json.dumps(
                {
                    "ok": True,
                    "device_dir": device_dir,
                    "device_root": DEVICE_ROOT,
                },
                ensure_ascii=True,
            )
        )
    elif args.format == "shell":
        print(f"DEVICE_DIR={device_dir}")
    else:
        print(device_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
